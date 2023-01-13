#ifndef _PE_CURL_EASY__H_20191120_
#define _PE_CURL_EASY__H_20191120_

#include "types.h"

#include <atomic>

#include <curl/curl.h>

namespace pe { namespace curl {

class Multi;
class StringList;

class Easy : noncopyable
{
public:
  using handle_t = CURL*;
  static const uint64_t REQUEST_TIMEOUT_MS = 300000;

public:
  Easy(bool verbose=false,
       uint64_t timeout=REQUEST_TIMEOUT_MS); //the maximum time in milliseconds that you allow the libcurl transfer operation to take
  Easy(handle_t h,
       bool verbose=false,
       uint64_t timeout=REQUEST_TIMEOUT_MS); //the maximum time in milliseconds that you allow the libcurl transfer operation to take
  virtual ~Easy();

public:
  void cancel();
  const char* errorText() const { return errorBuffer_; }
  operator handle_t() const { return handle(); }
  handle_t handle() const { return handle_; }
  size_t id() const { return id_; }
  void verbose(bool enable);
  bool verbose() const { return verbose_; }
  void xprogress(bool enable) const;

  //set maximum time in millisecond the request is allowed to take
  //the maximum time in milliseconds that you allow the libcurl transfer operation to take
  void timeout(uint64_t timeout) const;

public:
  void set(CURLoption option, bool arg) const
  {
    set(option, static_cast<long>(arg));
  }

  void set(CURLoption option, const StringList& commands) const;

  template<typename T>
  void set(CURLoption option, const T& arg) const
  {
    string text = "curl_easy_setopt(" + std::to_string((int)option) + ")";
    check_(curl_easy_setopt(handle(), option, arg), text.c_str());
  }

  //example:
  //   Easy* self = get<Easy*>(CURLINFO_PRIVATE);
  template<typename T>
  T get(CURLINFO info) const
  {
    T dst;
    return get(info, dst);
  }

  template<typename T>
  T& get(CURLINFO info, T& dst) const
  {
    string text = "curl_easy_getinfo(" + std::to_string((int)info) + ")";
    check_(curl_easy_getinfo(handle(), info, &dst), text.c_str());
    return dst;
  }

public:
  virtual void dispatch(const CURLMsg* msg)=0;

protected:
  virtual void debug(curl_infotype type, const char* data, size_t size);
  virtual int progress(curl_off_t downloadTotal,
                       curl_off_t downloaded,
                       curl_off_t uploadTotal,
                       curl_off_t uploaded);
  virtual size_t read(char* buf, size_t size);
  virtual size_t write(const char* buf, size_t size);
  virtual void canceling() {}

protected:
  void attach(Multi& multi);
  void detach(Multi& multi);
  void destroy();
  bool canceled() const { return canceled_; }

protected:
  static void check_(CURLcode err, const char* from);
  static handle_t check_(handle_t h);

private:
  static size_t createId();
  static int debug_(CURL* handle,
                    curl_infotype type,
                    char* data,
                    size_t size,
                    void* userptr);
  static int progress_(void* userdata,
                       curl_off_t dltotal,
                       curl_off_t dlnow,
                       curl_off_t ultotal,
                       curl_off_t ulnow);
  static size_t read_(char* buf,
                      size_t size,
                      size_t nitems,
                      void* userdata);
  static size_t write_(char* buf,
                       size_t size,
                       size_t nmemb,
                       void* userdata);

private:
  handle_t handle_;
  size_t id_;
  char errorBuffer_[CURL_ERROR_SIZE];
  std::atomic_bool canceled_{false};
  bool verbose_ = false;
};

} } //namespace pe { namespace curl {

#endif //_PE_CURL_EASY__H_20191120_