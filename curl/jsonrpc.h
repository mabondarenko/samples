#ifndef _PE_CURL_JSONRPC__H_20200307_
#define _PE_CURL_JSONRPC__H_20200307_

#include "httprequest.h"

#include <nlohmann/json.hpp>

namespace pe { namespace curl {

class JsonRPC : public HttpRequest
{
  using _Base = HttpRequest;
  using json = nlohmann::json;

public:
  explicit JsonRPC(const ServicePtr& service,
                   const string& url,
                   const string& method,
                   const json& params=json(),
                   bool verbose=false,
                   uint64_t timeout=REQUEST_TIMEOUT_MS);
  explicit JsonRPC(const ServicePtr& service,
                   const string& method,
                   const json& params=json(),
                   bool verbose=false,
                   uint64_t timeout=REQUEST_TIMEOUT_MS);

public:
  const json& error() const { return error_; }
  int64_t errorCode() const;
  string errorMessage() const;
  string errorStr(int indent=-1) const { return error().dump(indent); }
  const string& method() const { return method_; }
  const json& params() const { return params_; }
  void params(const json& value) { params_ = value; }
  bool successed() const { return successed_; }
  const string& version() const { return version_; }
  void version(const string& value) { version_ = value; }
  const json& result() const { return result_; }

public:
  virtual void run(ResultHandler&& handler);
  virtual bool run(int timeoutms=5000);

protected:
  virtual json request();

protected:
  virtual void done(int error, const char* text) override;
  virtual size_t write(const char* buf, size_t size) override;

private:
  void parse(const string& from) noexcept(false);

private:
  string method_;
  json params_;
  string buffer_;
  string version_ = "2.0";
  json result_;
  json error_;
  bool successed_ = false;
};

} } //namespace pe { namespace curl {

#endif //_PE_CURL_JSONRPC__H_20200307_