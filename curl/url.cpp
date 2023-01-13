#include "url.h"

#include <regex>

namespace pe { namespace curl {

Url::Url(void)
{
}

Url::Url(const string& from)
{
  parse(from, false);
}

bool Url::valid(void) const
{
  return !scheme_.empty() && !host_.empty();
}

string Url::server(void) const
{
  if (port_.empty())
    return scheme_ + "://" + host_;
  else
    return scheme_ + "://" + host_ + ":" + port_;
}

string Url::target(void) const
{
  if (path_.empty())
    return "/";

  return path_ + file_ + params_;
}

string Url::toString(void) const
{
  return server() + target();
}

void Url::patchPort(void)
{
  int port = getPort();
  if (port <= 0)
  {
    if (scheme_ == "https")
      port_ = "443";
    else if (scheme_ == "ftp")
      port_ = "21";
    else
      port_ = "80"; //http
  }
}

bool Url::parse(const string& src, bool force)
{
  if (force)
    invalidate();

  static std::regex expression(
    //   proto                 host               port
    "^(\?:([^:/\?#]+)://)\?(\\w+[^/\?#:]*)(\?::(\\d+))\?"
    //   path                  file       parameters
    "(/\?(\?:[^\?#/]*/)*)\?([^\?#]*)\?(\\\?(.*))\?"
  );

  std::smatch m;
  if (std::regex_match(src, m, expression))
  {
    scheme_ = m[1];
    host_   = m[2];
    port_   = m[3];
    path_   = m[4];
    file_   = m[5];
    params_ = m[6];

    if (scheme_.empty())
      scheme_ = "https";

    patchPort();

    return valid();
  }

  return false;
}

int Url::getPort(void) const
{
  return std::atoi(port_.c_str());
}

void Url::invalidate(void)
{
  scheme_.clear();
  host_.clear();
  port_.clear();
  path_.clear();
  file_.clear();
  params_.clear();
}

bool Url::isSSL(void) const
{
  return valid() && scheme()=="https";
}

} } //namespace pe { namespace curl {

