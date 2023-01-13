#ifndef _PE_CURL_SLIST__H_20191125_
#define _PE_CURL_SLIST__H_20191125_

#include "types.h"

#include <curl/curl.h>

namespace pe { namespace curl {

class StringList : noncopyable
{
public:
  StringList() {}
  StringList(const char* item) { append(item); }
  StringList(const string& item) { append(item); }
  StringList(const string_list_t& items) { append(items); }
  ~StringList() { clear(); }

public:
  const curl_slist* get() const { return slist_; }
  operator const curl_slist* () const { return get(); }

public:
  void append(const char* item)
  {
    curl_slist* tmp = curl_slist_append(slist_, item);
    if (!tmp)
      throw std::runtime_error("curl_slist_append() failed");
    slist_ = tmp;
  }

  void append(const string& item) { return append(item.c_str()); }
  void append(const string_list_t& items)
  {
    for (const auto& i: items)
      append(i);
  }

  void clear()
  {
    if (slist_)
    {
      curl_slist_free_all(slist_);
      slist_ = nullptr;
    }
  }

private:
  curl_slist* slist_ = nullptr;
};

} } //namespace pe { namespace curl {

#endif //#ifndef _PE_CURL_SLIST__H_20191125_