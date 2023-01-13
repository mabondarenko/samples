#include "reqbase.h"

#include "tools/url.h"

namespace pe { namespace scgi {

RequestBase::json RequestBase::toJson() const
{
  json j;
  j["id"] = id();
  j["uri"] = uri();
  j["method"] = method_name();

  if (!sessionId().empty())
    j["sessionId"] = sessionId();

  if (!data().empty() && !data().is_null())
    j["data"] = data();

  return j;
}

void RequestBase::fromJson(const json& from)
{
  set_id(from.at("id")); //required
  set_method(from.at("method").get<string>()); //required
  set_uri(from.at("uri")); //required

  if (present(from, "data"))
    set_data(from.at("data"));
  else
    set_data(json());

  if (present(from, "sessionId"))
    set_sessionId(from.at("sessionId"));
  else
    set_sessionId(string());
}

const RequestBase::method_map_t& RequestBase::method_map()
{
  static const method_map_t m = {
    { Method::kUNKNOWN, "UNKNOWN" },
    { Method::kGET, "GET" },
    { Method::kPOST, "POST" },
    { Method::kPUT, "PUT" },
    { Method::kPATCH, "PATCH" },
    { Method::kDELETE, "DELETE" },
    { Method::kEVENT, "EVENT" }
  };
  return m;
}

void RequestBase::set_uri(const string& value)
{
  uri_.reserve(value.size());

  bool sprev = false;
  for(auto c: value)
  {
    if (c=='/' || c=='\\')
    {
      if (!sprev)
      {
        uri_.push_back(c);
        sprev = true;
      }
    }
    else
    {
      if (uri_.size()==0 )
        uri_.push_back('/');

      uri_.push_back(c);
      sprev = false;
    }
  }

  auto p = param();
  p.item = "";
}

RequestBase::Param RequestBase::param() const
{
  return Param(uri());
}

RequestBase::Param::Param(const string& uri)
{
  static const string host = "cgi://local";
  tools::Url u(host + uri);

  if (u.valid())
  {
    param = u.path();
    item = u.file();
  }
}

} } //namespace pe { namespace scgi {
