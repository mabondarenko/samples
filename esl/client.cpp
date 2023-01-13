#include "client.h"

#include <boost/chrono.hpp>
#include <boost/format.hpp>
#include <nlohmann/json.hpp>

#include "command.h"
#include "event.h"
#include "tools/str.h"
#include "trace.h"

namespace pe { namespace esl {

PE_DECLARE_LOG_CHANNEL(THISLOG, "esl.client: ");

IClient::pointer_t IClient::create(boost::asio::io_service& ios,
                                   const string& password,
                                   const string& host,
                                   uint16_t port)
{
  return std::make_shared<Client>(ios, password, host, port);
}

Client::Client(boost::asio::io_service& ios,
               const string& password,
               const string& host,
               uint16_t port)
  : ios_(ios)
  , host_(host)
  , password_(password)
  , port_(port)
{
  PE_DEBUG(THISLOG << "created...");

  reset();
}

Client::~Client()
{
  PE_DEBUG(THISLOG << "destroying...");
}

void Client::eventMask(const string_set_t& value, bool append)
{
  guard_t g(mtxevents_);

  if (append)
  {
    for(const auto& v: value)
      eventmask_.insert(v);
  }
  else
  {
    eventmask_ = value;
  }
}

Client::EventSignal& Client::onevent(const string& evtname)
{
  guard_t g(mtxevents_);
  return feventmap_[evtname];
}

Client::EventSignal* Client::eventSignal(const string& evtname)
{
  guard_t g(mtxevents_);

  auto it = feventmap_.find(evtname);
  if (it!=feventmap_.end())
    return &it->second;

  return nullptr;
}

void Client::start(const string_set_t& evtmask)
{
  if (!stopped_.exchange(false))
  {
    PE_ERROR(THISLOG << "already started...");
    return;
  }

  PE_DEBUG(THISLOG << "starting...");

  eventMask(evtmask);

  auto self = shared_from_this();
  ios().post([self, this](){ poll(); });
}

void Client::stop()
{
  if (stopped_.exchange(true))
  {
    PE_ERROR(THISLOG << "already stopped...");
    return;
  }

  PE_DEBUG(THISLOG << "stopping...");
}

void Client::poll()
{
  PE_DEBUG(THISLOG << "poll thread started...");

  while(!stopped())
  {
    if (connect() && handle()->connected)
    {
      onConnected();
      recv();
    }
    else
    {
      guard_t g(mtxstop_);
      cndstop_.wait_for(g, boost::chrono::milliseconds(1000));
    }
  }

  disconnect();

  PE_DEBUG(THISLOG << "poll thread stopped...");
}

void Client::recv()
{
  bool fexit = false;

  while(!stopped() && handle()->connected && !fexit)
  {
    esl_event_t* evt = nullptr;

    auto st = esl_recv_event_timed(handle(), 10, 1, &evt);

    switch (st)
    {
    case ESL_BREAK: //timeout
      break;
    case ESL_SUCCESS:
      if (evt)
        fexit = !process(evt);
      break;
    case ESL_FAIL:
      PE_ERROR(THISLOG << "recv: ESL_FAIL");
      fexit = true;
      break;
    case ESL_DISCONNECTED:
      PE_ERROR(THISLOG << "recv: ESL_DISCONNECTED");
      fexit = true;
      break;
    case ESL_GENERR:
      PE_ERROR(THISLOG << "recv: ESL_GENERR");
      fexit = true;
      break;
    }
  }

  if (!handle()->connected || fexit)
    onDisconnected();
}

void Client::onConnected()
{
  PE_INFO(THISLOG << "connected to " << host() << ":" << port());

  fonconnect_(shared_from_this());

  subscribe();
}

void Client::onDisconnected()
{
  PE_INFO(THISLOG << "disconnected from " << host() << ":" << port());

  fondisconnect_(shared_from_this());
}

void Client::subscribe()
{
  static const esl_event_type_t etype = ESL_EVENT_TYPE_JSON;

  auto evts = eventList();

  if (evts.empty())
  {
    PE_WARNING(THISLOG << "subscribe event mask is empty");
  }
  else if (esl_events(handle(), etype, evts.c_str())!=ESL_SUCCESS)
  {
    PE_ERROR(THISLOG << "failed to subscribe on events: " << evts);
  }
  else
  {
    PE_DEBUG(THISLOG << "subscribed on events: " << evts);
  }
}

string Client::eventList() const
{
  static const string sSEP(" ");
  return tools::str::toList(eventMask(), sSEP);
}

Client::EventPtr Client::parse(esl_event_t* evt)
{
  const char *type = esl_event_get_header(evt, "content-type");

  if (!type)
    throw exceptions::runtime_error("unknown (NULL) ESL event type");

  char* str = nullptr;
  if (esl_event_serialize_json(evt, &str)!=ESL_SUCCESS)
    throw exceptions::runtime_error("failed to serialize ESL event");

  try
  {
    using json = nlohmann::json;

    auto j = json::parse(str);
    if (strcasecmp(type, "text/event-json")==0)
    {
      string body = j.at("_body");
      return std::make_shared<Event>(body, type);
    }
    else
      return std::make_shared<Event>(j, type);
  }
  catch(const std::exception& c)
  {
    PE_ERROR(THISLOG << "invalid JSON event content:\n'" << str << "'");
    throw;
  }
}

bool Client::process(esl_event_t* evt)
{
  try
  {
    return dispatch(parse(evt));
  }
  catch(const std::exception& e)
  {
    PE_ERROR(THISLOG << "failed to parse event: " << e.what());
  }

  return true;
}

bool Client::dispatch(const EventPtr& evt)
{
  if (evt->isDisconnectNotice())
  {
    PE_INFO(THISLOG << "disconnect notice received: " << evt->bodyStr());
    return false;
  }

  try
  {
    EventSignal* f = eventSignal(evt->name());
    if (f)
      (*f)(shared_from_this(), evt);
  }
  catch(const std::exception& e)
  {
    PE_ERROR(THISLOG << "failed to disptach event: " << e.what() << "\nevent:\n" << evt->toString());
  }

  return true;
}

bool Client::connect(uint32_t timeout)
{
  guard_t g(mtxsend_);

  reset();

  return esl_connect_timeout(handle(),
                             host().c_str(),
                             port(),
                             nullptr,
                             password().c_str(),
                             timeout)==ESL_SUCCESS;
}

bool Client::disconnect()
{
  bool ret = true;

  PE_DEBUG(THISLOG << "trying to disconnect...");

  guard_t g(mtxsend_);

  if (handle()->connected)
  {
    PE_DEBUG(THISLOG << "disconnecting...");

    ret = esl_disconnect(handle())==ESL_SUCCESS;

    if (ret && !handle()->connected)
      onDisconnected();
  }
  else
  {
    PE_DEBUG(THISLOG << "client is not connected");
  }

  reset();

  return ret;
}

void Client::reset()
{
  memset(handle(), 0, sizeof(esl_handle_t));
}

Client::EventPtr Client::send(const Command& cmd, u_int32_t timeoutms)
{
  return send(cmd.str(), timeoutms);
}

Client::EventPtr Client::send(const string& cmd, u_int32_t timeoutms)
{
  return send(cmd.c_str(), timeoutms);
}

Client::EventPtr Client::send(const char* cmd, u_int32_t timeoutms)
{
  guard_t g(mtxsend_);

  if (!handle()->connected)
    throw exceptions::runtime_error("disconnected from freeswitch");

  PE_DEBUG(THISLOG << "trying to send command '" << cmd << "'...");

  auto status = esl_send_recv_timed(handle(), cmd, timeoutms);
  if (status!=ESL_SUCCESS)
  {
    auto err = (boost::format("failed to send command '%1%', status=%2%, err=%3%, errmsg='%4%'")
                % cmd % status % handle()->errnum % handle()->err).str();
    throw exceptions::runtime_error(err);
  }

  try
  {
    auto evt = parse(handle()->last_sr_event);

#if(1)
    if (evt->isErrorResponse())
    {
      PE_ERROR(THISLOG << "freeswitch failed to execute command '" << cmd << "':\n" << evt->toString());
    }
    else
    {
      PE_DEBUG(THISLOG << "received response on command '" << cmd << "':\n" << evt->toString());
    }
#endif

    return evt;
  }
  catch(const std::exception& e)
  {
    PE_ERROR(THISLOG << "failed to parse handle()->last_sr_event: " << e.what());
    throw;
  }
}

} } //namespace pe { namespace esl {