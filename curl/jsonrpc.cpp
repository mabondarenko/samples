#include "jsonrpc.h"

#include "tools/jsonhelper.h"
#include "syncwait.h"

#include "trace.h"

namespace pe { namespace curl {

PE_DECLARE_LOG_CHANNEL(JRPCLOG, "curl.jsonrpc");
#define THISLOG JRPCLOG << "[" << id() << "]: "

#ifdef _DEBUG
  #define BODY_JSON_PRETTY_INDENT 2
#else
  #define BODY_JSON_PRETTY_INDENT (-1)
#endif

JsonRPC::JsonRPC(const ServicePtr& service,
                 const string& url,
                 const string& method,
                 const json& params,
                 bool verbose,
                 uint64_t timeout)
  : _Base(service, url, verbose, timeout)
  , method_(method)
  , params_(params)
{
}

JsonRPC::JsonRPC(const ServicePtr& service,
                 const string& method,
                 const json& params,
                 bool verbose,
                 uint64_t timeout)
  : JsonRPC(service, string(), method, params, verbose, timeout)
{
}

void JsonRPC::run(ResultHandler&& handler)
{
  contentType("application/json");

  auto req = request().dump(BODY_JSON_PRETTY_INDENT);

  if (verbose())
  {
    PE_DEBUG(THISLOG << "sending:\n" << req);
  }

  return POST(std::move(handler), req);
}

bool JsonRPC::run(int timeoutms)
{
  PE_TRACE(THISLOG << "run()");

  auto s = std::make_shared<Synchro>(shared_from_this());

  run([s](const pointer_t&, int, const char*) {
    s->finish();
  });

  if (!s->wait(timeoutms))
  {
    PE_ERROR(THISLOG << "request timeout...");
  }

  return successed();
}

JsonRPC::json JsonRPC::request()
{
  json req = {
    { "id", id() },
    { "jsonrpc", version() },
    { "method", method() }
  };

  if (!params().is_null())
    req["params"] = params();

  return req;
}

int64_t JsonRPC::errorCode() const
{
  static string sCODE("code");
  return jhlp::getValue<int64_t>(error(), sCODE, 0);
}

string JsonRPC::errorMessage() const
{
  static string sMESSAGE("message");
  return jhlp::getValue<string>(error(), sMESSAGE);
}

void JsonRPC::done(int error, const char* text)
{
  try
  {
    if (!error)
      parse(buffer_);
  }
  catch(const std::exception& e)
  {
    PE_ERROR(THISLOG << "failed to parse response from URL: '" << url() << "'...");
    return _Base::done(CURLE_WEIRD_SERVER_REPLY, e.what());
  }

  return _Base::done(error, text);
}

size_t JsonRPC::write(const char* buf, size_t size)
{
  buffer_.append(buf, size);
  return size;
}

void JsonRPC::parse(const string& from)
{
  int rcode = responseCode();

  if (verbose())
  {
    PE_DEBUG(THISLOG << "parsing response for '" << url() << "', HTTP code: " << rcode);
  }

  if (rcode!=200)
    throw std::runtime_error("bad HTTP response code: " + std::to_string(rcode));

  if (contentType()!="application/json")
    throw std::runtime_error("invalid Content-Type");

  auto resp = json::parse(from);

  if (verbose())
  {
    static const size_t max_text_length = 5 * 1024;
    auto text = resp.dump(2);
    if (text.length()<=max_text_length)
    {
      PE_DEBUG(THISLOG << "server response:\n" << text);
    }
    else
    {
      text.resize(max_text_length);
      PE_DEBUG(THISLOG << "server response [truncated]:\n" << text);
    }
  }

  const auto& jrpcvers = resp.find("jsonrpc");
  if (jrpcvers==resp.end())
    throw std::runtime_error("can't find JSON-RPC version");

  if (!jrpcvers.value().is_string() || jrpcvers.value()!=version())
    throw std::runtime_error("bad JSON-RPC version");

  const auto& jrpcid = resp.find("id");
  if (jrpcid==resp.end())
    throw std::runtime_error("can't find JSON-RPC id");

  if (!jrpcid.value().is_number() || jrpcid.value()!=id())
    throw std::runtime_error("invalid JSON-RPC id");

  const auto& result = resp.find("result");
  const auto& error = resp.find("error");

  if (result!=resp.end() && error!=resp.end())
  {
    throw std::runtime_error("ambiguous JSON-RPC response object: result and error");
  }
  else if (error!=resp.end())
  {
    if (error.value().is_null())
      throw std::runtime_error("bad JSON-RPC error object");

    error_ = error.value();
  }
  else if (result!=resp.end())
  {
    result_ = result.value();
    successed_ = true;
  }
  else
    throw std::runtime_error("bad JSON-RPC response: no result or error");
}

} } //namespace pe { namespace curl {