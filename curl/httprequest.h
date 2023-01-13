#ifndef _PE_CURL_HTTPREQUEST__H_20200307_
#define _PE_CURL_HTTPREQUEST__H_20200307_

#include "transfer.h"
#include "slist.h"

namespace pe { namespace curl {

class HttpRequest : public Transfer
{
  using _Base = Transfer;

public:
  HttpRequest(const ServicePtr& service,
              const string& url=string(),
              bool verbose=false,
              uint64_t timeout=REQUEST_TIMEOUT_MS);

public:
  void addHeader(const string& header) { headers_.append(header); }
  void addHeaders(const string_list_t& headers);
  void contentType(const string& content);
  string contentType() const;
  long responseCode() const;

public:
  static string encode(const string& value);
  static string decode(const string& value);

public:
  virtual void exec(ResultHandler&& handler) override;
  virtual void GET(ResultHandler&& handler);
  virtual void POST(ResultHandler&& handler);
  virtual void POST(ResultHandler&& handler, const string& data);
  virtual void POST(ResultHandler&& handler, const buffer& data);

protected:
  virtual const char* data() const { return nullptr; }
  virtual size_t dataSize() const { return 0; }

private:
  StringList headers_;
};

} } //namespace pe { namespace curl {

#endif //_PE_CURL_HTTPREQUEST__H_20200307_