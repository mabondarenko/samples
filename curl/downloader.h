#ifndef _PE_CURL_DOWNLOADER__H_20191126_
#define _PE_CURL_DOWNLOADER__H_20191126_

#include "transfer.h"

#include <fstream>

namespace pe { namespace curl {

class Downloader : public Transfer
{
  using _Base = Transfer;

public:
  Downloader(const ServicePtr& service,
             const string& url,
             const string& file,
             uint64_t timeout=REQUEST_TIMEOUT_MS);

protected:
  virtual void done(int error, const char* text) override;
  virtual size_t write(const char* buf, size_t size) override;

private:
  bool open();
  void close(int error);

private:
  string file_;
  string tmpfile_;
  std::ofstream out_;
};

} } //namespace pe { namespace curl {

#endif //#ifndef _PE_CURL_DOWNLOADER__H_20191126_