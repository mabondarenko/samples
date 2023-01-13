#include "multi.h"
#include "easy.h"

#include <boost/format.hpp>

#include "trace.h"

namespace pe { namespace curl {

PE_DECLARE_LOG_CHANNEL(MULTILOG, "curl.multi");
#define THISLOG MULTILOG << "[" << name() << "]: "

Multi::Multi(const string& _name)
  : Multi(curl_multi_init(), _name)
{
}

Multi::Multi(handle_t h, const string& _name)
  : name_(_name)
  , handle_(check_(h))
{
  set(CURLMOPT_SOCKETFUNCTION, action_);
  set(CURLMOPT_SOCKETDATA, this);

  set(CURLMOPT_TIMERFUNCTION, setTimer_);
  set(CURLMOPT_TIMERDATA, this);
}

Multi::~Multi()
{
  destroy();
}

void Multi::destroy()
{
  if (handle())
  {
    curl_multi_cleanup(handle());
    handle_ = nullptr;
  }
}

void Multi::check_(CURLMcode err, const char* from)
{
  if (err!=CURLM_OK)
  {
    boost::format fmt("%1%: '%2%'");
    string text = (fmt % from % curl_multi_strerror(err)).str();
    throw exceptions::runtime_error(text);
  }
}

Multi::handle_t Multi::check_(handle_t h)
{
  if (!h)
    throw std::invalid_argument("invalid MULTI handle");

  return h;
}

int Multi::action(curl_socket_t s, int evbitmask)
{
  int running = 0;
  try
  {
    guard_t g(mutex_);
    check_(curl_multi_socket_action(handle(), s, evbitmask, &running), "curl_multi_socket_action()");
  }
  catch(const std::exception& e)
  {
    PE_ERROR(THISLOG << "throwed in action(): socket=" << s
                     << ", mask=" << evbitmask
                     << ", running=" << running
                     << ", exception: " << e.what());
    throw;
  }

  return running;
}

void Multi::assign(const curl_socket_t s, void* data)
{
  guard_t g(mutex_);
  check_(curl_multi_assign(handle(), s, data), "curl_multi_assign()");
}

void Multi::attach(const Easy& easy)
{
  easy.set(CURLOPT_OPENSOCKETFUNCTION, socket_);
  easy.set(CURLOPT_OPENSOCKETDATA, this);

  easy.set(CURLOPT_SOCKOPTFUNCTION, options_);
  easy.set(CURLOPT_SOCKOPTDATA, this);

  easy.set(CURLOPT_CLOSESOCKETFUNCTION, close_);
  easy.set(CURLOPT_CLOSESOCKETDATA, this);

  guard_t g(mutex_);
  check_(curl_multi_add_handle(handle(), easy.handle()), "curl_multi_add_handle()");
}

void Multi::detach(const Easy& easy)
{
  //It is fine to remove a handle at any time during a transfer,
  //just not from within any libcurl callback function.
  guard_t g(mutex_);
  check_(curl_multi_remove_handle(handle(), easy.handle()), "curl_multi_remove_handle()");
}

void Multi::dispatch(const CURLMsg* msg, void* peasy)
{
  if (peasy)
  {
    Easy* easy = reinterpret_cast<Easy*>(peasy);
    try
    {
      easy->dispatch(msg);
    }
    catch(const std::exception& e)
    {
      PE_ERROR(THISLOG << "failed to dispatch message to [" << easy->id() << "]: " << e.what());
    }
  }
}

void Multi::process(curl_socket_t s, int evbitmask)
{
  try
  {
    if (action(s, evbitmask)<=0)
    {
#ifdef _CURL_DEBUG
      PE_TRACE(THISLOG << "no active transfers, canceling timer...");
#endif
      cancelTimer();
    }
  }
  catch(const std::exception& e)
  {
    PE_ERROR(THISLOG << "exception while proceeding socket action: " << e.what());
  }

  poll();
}

void Multi::poll()
{
  //PE_TRACE(THISLOG << "poll()");

  int msgleft = 0;

  guard_t g(mutex_);
  while(auto msg = curl_multi_info_read(handle(), &msgleft))
  {
    void* easy = nullptr;
    curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &easy);
    dispatch(msg, easy);
  }
}

int Multi::timeout()
{
  return action(CURL_SOCKET_TIMEOUT, 0);
}

void Multi::timer()
{
  try
  {
    timeout();
  }
  catch(const std::exception& e)
  {
    PE_ERROR(THISLOG << "exception while proceeding timer: " << e.what());
  }

  poll();
}

int Multi::action_(CURL* easy,
                   curl_socket_t s,
                   int what,
                   void* userp,
                   void* socketp)
{
  auto self = reinterpret_cast<Multi*>(userp);
  if (!self)
    return -1;

  return self->action(easy, s, what, socketp);
}

int Multi::close_(void* userp, curl_socket_t s)
{
  auto self = reinterpret_cast<Multi*>(userp);
  if (!self)
    return 1;

  return self->close(s);
}

int Multi::options_(void* userp, curl_socket_t s, curlsocktype purpose)
{
  auto self = reinterpret_cast<Multi*>(userp);
  if (!self)
    return CURL_SOCKOPT_ERROR;

  return self->options(s, purpose);
}

int Multi::setTimer_(CURLM* multi,
                     long timeout_ms,
                     void* userp)
{
  PE_UNUSED(multi);

  auto self = reinterpret_cast<Multi*>(userp);
  if (!self)
    return -1;

  PE_ASSERT(multi==self->handle());

  return self->setTimer(timeout_ms);
}

curl_socket_t Multi::socket_(void* userp, curlsocktype purpose, curl_sockaddr* address)
{
  auto self = reinterpret_cast<Multi*>(userp);
  if (!self)
    return CURL_SOCKET_BAD;

  return self->socket(purpose, address);
}

} } //namespace pe { namespace curl {