#ifndef _PE_CURL_RMFILES__H_20191125_
#define _PE_CURL_RMFILES__H_20191125_

#include "transfer.h"

#include "slist.h"

namespace pe { namespace curl {

class RemoveFiles : public Transfer
{
  using _Base = Transfer;

public:
  RemoveFiles(const ServicePtr& service,
              const string& url,
              const string& file,
              uint64_t timeout=REQUEST_TIMEOUT_MS)
    : RemoveFiles(service, url, string_list_t{file}, timeout)
  {
  }

  RemoveFiles(const ServicePtr& service,
              const string& url,
              const string_list_t& files,
              uint64_t timeout=REQUEST_TIMEOUT_MS)
    : _Base(service, url, timeout)
  {
    for (const auto& f: files)
      commands_.append("rm '" + f + "'");

    quote(commands_);

    noBODY();
  }

private:
  StringList commands_;
};

} } //namespace pe { namespace curl {

#endif //#ifndef _PE_CURL_RMFILES__H_20191125_