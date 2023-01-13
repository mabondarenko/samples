#include "httprequest.h"
#include "urilite.h"

#include "trace.h"

namespace pe { namespace curl {

PE_DECLARE_LOG_CHANNEL(TRFLOG, "curl.http");
#define THISLOG TRFLOG << "[" << id() << "]: "

HttpRequest::HttpRequest(const ServicePtr& service,
                         const string& url,
                         bool verbose,
                         uint64_t timeout)
  : _Base(service, url, verbose, timeout)
{
}

void HttpRequest::addHeaders(const string_list_t& headers)
{
  for (const auto& h: headers)
    addHeader(h);
}

void HttpRequest::exec(ResultHandler&& handler)
{
  set(CURLOPT_HTTPHEADER, headers_);

  return _Base::exec(std::move(handler));
}

void HttpRequest::GET(ResultHandler&& handler)
{
  PE_TRACE(THISLOG << "sending GET '" << url() << "'...");

  set(CURLOPT_HTTPGET, true);

  return exec(std::move(handler));
}

void HttpRequest::POST(ResultHandler&& handler)
{
  PE_TRACE(THISLOG << "sending POST '" << url() << "'...");

  set(CURLOPT_HTTPPOST, true);
  set(CURLOPT_POSTFIELDSIZE_LARGE, dataSize());
  set(CURLOPT_POSTFIELDS, data());

  return exec(std::move(handler));
}

void HttpRequest::POST(ResultHandler&& handler, const string& data)
{
  PE_TRACE(THISLOG << "sending POST '" << url() << "'...");

  set(CURLOPT_HTTPPOST, true);
  set(CURLOPT_POSTFIELDSIZE_LARGE, data.length());
  set(CURLOPT_COPYPOSTFIELDS, data.data());

  return exec(std::move(handler));
}

void HttpRequest::POST(ResultHandler&& handler, const buffer& data)
{
  PE_TRACE(THISLOG << "sending POST '" << url() << "'...");

  set(CURLOPT_HTTPPOST, true);
  set(CURLOPT_POSTFIELDSIZE_LARGE, data.size());
  set(CURLOPT_COPYPOSTFIELDS, data.data());

  return exec(std::move(handler));
}

void HttpRequest::contentType(const string& content)
{
  static const string sCONTENT_TYPE("Content-Type: ");
  addHeader(sCONTENT_TYPE + content);
}

string HttpRequest::contentType() const
{
  try
  {
    return get<const char*>(CURLINFO_CONTENT_TYPE);
  }
  catch(const std::exception& e)
  {
    PE_ERROR(THISLOG << "failed to get Content-Type: " << e.what());
  }

  return string();
}

long HttpRequest::responseCode() const
{
  try
  {
    return get<long>(CURLINFO_RESPONSE_CODE);
  }
  catch(const std::exception& e)
  {
    PE_ERROR(THISLOG << "failed to get response code: " << e.what());
  }

  return 0;
}

string HttpRequest::encode(const string& value)
{
  return urilite::encoder<>::encode(value);
}

string HttpRequest::decode(const string& value)
{
  return urilite::encoder<>::decode(value);
}

} } //namespace pe { namespace curl {