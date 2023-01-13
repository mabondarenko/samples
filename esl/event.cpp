#include "event.h"

#include "tools/jsonhelper.h"
#include "tools/str.h"

namespace pe { namespace esl {

Event::Event(const string& _from, const char* _contentType)
  : Event(json::parse(_from), _contentType)
{
}

Event::Event(const json& _from, const char* _contentType)
  : data_(_from)
  , name_(data_.at("Event-Name"))
  , contentType_(_contentType)
{
}

string Event::value(const char* field, const string& def) const
{
  return jhlp::getValue(data_, field, def);
}

const Event::json& Event::body() const
{
  auto it = data_.find("_body");
  if (it!=data_.end())
    return it.value();

  static const json _body;
  return _body;
}

string Event::bodyStr() const
{
  const auto& b = body();
  if (b.is_string())
    return b.get<string>();

  return b.dump(2);
}

bool Event::isAPIResponse() const
{
  return strcasecmp(contentType_.c_str(), "api/response")==0;
}

bool Event::isCommandReply() const
{
  return strcasecmp(contentType_.c_str(), "command/reply")==0;
}

bool Event::isDisconnectNotice() const
{
  return strcasecmp(contentType_.c_str(), "text/disconnect-notice")==0;
}

bool Event::checkError() const
{
  if (isAPIResponse())
  {
    const auto& b = body();
    if (b.is_string())
    {
      auto str = bodyStr();

      if (tools::str::startsWith(str, "-ERR "))
        return true;

      if (tools::str::startsWith(str, "-USAGE:"))
        return true;
    }
  }
  else if (isCommandReply())
    return !tools::str::startsWith(value("Reply-Text"), "+OK ");

  return false;
}

bool Event::isErrorResponse() const
{
  if (!errorDetected_)
  {
    errorFlag_ = checkError();
    errorDetected_ = true;
  }
  return errorFlag_;
}

} } //namespace pe { namespace esl {