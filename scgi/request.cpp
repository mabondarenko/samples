#include "request.h"

#include "connection.h"
#include "response.h"

#include "trace.h"

namespace pe { namespace scgi {

PE_DECLARE_LOG_CHANNEL(REQUEST_LOG, "scgi.request[");
#define THISLOG REQUEST_LOG << id() << "]: "

Request::Request(const ConnectionPtr& _connection, const buffer& msg)
  : connection_(_connection)
{
  fromJson(json::parse(msg));
}

Request::json Request::toJson() const
{
  auto j = _Base::toJson();
  j["from"] = from();
  return j;
}

void Request::fromJson(const json& from)
{
  _Base::fromJson(from);
  set_from(from.at("from"));
}

void Request::reply(int code, const string& status)
{
  auto con = connection();
  if (!con)
  {
    //connection closed
    return;
  }

  try
  {
    con->write(Message(Response(*this, code, status).toString()));
  }
  catch (std::exception& e)
  {
    PE_ERROR(THISLOG << "failed to send reply: " << e.what());
  }
}

void Request::reply(const json& data)
{
  auto con = connection();
  if (!con)
  {
    //connection closed
    return;
  }

  try
  {
    con->write(Message(Response(*this, data).toString()));
  }
  catch (std::exception& e)
  {
    PE_ERROR(THISLOG << "failed to send reply: " << e.what());
  }
}

} } //namespace pe { namespace scgi {
