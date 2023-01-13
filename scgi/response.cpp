#include "response.h"

namespace pe { namespace scgi {

Response::Response(const RequestBase& from, const json& data)
  : code_(200)
  , status_("OK")
{
  init(from);
  set_data(data);
}

Response::Response(const RequestBase& from, int _code, const string& _status)
  : code_(_code)
  , status_(_status)
{
  init(from);
}

void Response::init(const RequestBase& from)
{
  set_id(from.id());
  set_uri(from.uri());
  set_method(from.method());
}

Response::json Response::toJson() const
{
  auto j = _Base::toJson();
  j["code"] = code();
  j["status"] = status();
  return j;
}

void Response::fromJson(const json& from)
{
  _Base::fromJson(from);
  set_code(from.at("code"));
  set_status(from.at("status"));
}

} } //namespace pe { namespace scgi {
