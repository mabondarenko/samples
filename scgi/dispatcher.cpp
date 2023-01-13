#include "dispatcher.h"

#include <regex>

#include "../trace.h"

#include "connection.h"
#include "request.h"

namespace pe { namespace scgi {

PE_DECLARE_LOG_CHANNEL(THISLOG, "scgi.dispatcher: ");

Dispatcher::Dispatcher(boost::asio::io_service& _ios)
  : ios_(_ios)
  , strand_(ios())
{
  PE_DEBUG(THISLOG << "created...");
}

Dispatcher::~Dispatcher()
{
  PE_DEBUG(THISLOG << "destroying...");
}

bool Dispatcher::onMessage(const ConnectionPtr& connection, const buffer& msg)
{
  try
  {
    process(std::make_shared<Request>(connection, msg));
  }
  catch (std::exception& e)
  {
    string str(msg.begin(), msg.end());
    PE_ERROR(THISLOG << "BAD message from connection[" << connection->id() << "] received:\n\n'"
                      << str << "'\n"
                      << "parsing failed with: " << e.what());
  }

  return true;
}

void Dispatcher::process(const RequestPtr& request)
{
  auto self = shared_from_this();
  strand_.post([this, self, request]() { exec(request); });
}

void Dispatcher::exec(const RequestPtr& request)
{
  if (auto h = find(request->uri()))
    h(request);
  else
  {
    PE_TRACE(THISLOG << "handler for " << request->method_name()
                    << " '" << request->uri() << "' not found!");
    request->reply(404, "Not found");
  }
}

void Dispatcher::add(const string& pattern, const RequestHandler& handler)
{
  if (handler)
  {
    guard_t (mtx_);
    handlers_[createPattern(pattern)] = handler;
  }
}

void Dispatcher::remove(const string& pattern)
{
  auto exp = createPattern(pattern);

  guard_t (mtx_);
  handlers_.erase(exp);
}

void Dispatcher::cancel()
{
  guard_t (mtx_);
  handlers_.clear();
}

string Dispatcher::createPattern(const string& from)
{
  return std::regex_replace("^" + from + "$", std::regex("\\/"), "\\/");
}

Dispatcher::RequestHandler Dispatcher::find(const string& uri) const
{
  guard_t (mtx_);
  for(const auto& h: handlers_)
  {
    std::regex exp(h.first);
    if (std::regex_match(uri, exp))
      return h.second;
  }

  return RequestHandler();
}

} } //namespace pe { namespace scgi {
