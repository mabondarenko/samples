#ifndef _PE_API_CURL_URL__H_20180801_
#define _PE_API_CURL_URL__H_20180801_

#include "types.h"

namespace pe { namespace curl {

class Url
{
public:
  Url(void);
  Url(const string& from);

public:
  bool valid(void) const;
  operator bool(void) const { return valid(); }
  const string& scheme(void) const { return scheme_; }
  const string& host(void) const { return host_; }
  const string& port(void) const { return port_; }
  const string& file(void) const { return file_; }
  const string& params(void) const { return params_; }
  string server(void) const;
  string target(void) const;
  string toString(void) const;
  int getPort(void) const;
  bool isSSL(void) const;

private:
  bool parse(const string& src, bool force=true);
  void invalidate(void);
  void patchPort(void);

private:
  string scheme_; //protocol
  string host_;
  string port_;
  string path_;
  string file_;
  string params_;
};

} } //namespace pe { namespace curl {

#endif //_PE_API_CURL_URL__H_20180801_
