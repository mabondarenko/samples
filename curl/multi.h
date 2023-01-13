#ifndef _PE_CURL_MULTI__H_20191120_
#define _PE_CURL_MULTI__H_20191120_

#include "types.h"

#include <curl/curl.h>
#include <boost/thread/recursive_mutex.hpp>

namespace pe { namespace curl {

class Easy;

class Multi : noncopyable
{
public:
  using handle_t = CURLM*;

public:
  Multi(const string& name);
  Multi(handle_t h, const string& name);
  ~Multi();

public:
  void attach(const Easy& easy);
  void detach(const Easy& easy);
  operator handle_t() const { return handle(); }
  handle_t handle() const { return handle_; }
  const string& name() const { return name_; }

protected:
  virtual int action(CURL* easy,
                     curl_socket_t s,
                     int what,
                     void* socketp)=0;
  virtual void cancelTimer()=0;
  virtual int close(curl_socket_t s)=0;
  virtual int options(curl_socket_t s, curlsocktype purpose)=0;
  virtual int setTimer(long timeout_ms)=0;
  virtual curl_socket_t socket(curlsocktype purpose, curl_sockaddr* address)=0;

protected:
  int action(curl_socket_t s, int evbitmask);
  void assign(const curl_socket_t s, void* data);
  void dispatch(const CURLMsg* msg, void* easy);
  void destroy();
  void poll();
  void process(curl_socket_t s, int evbitmask);
  int timeout();
  void timer();

protected:
  void set(CURLMoption option, bool arg) const
  {
    set(option, static_cast<long>(arg));
  }

  template<typename T>
  void set(CURLMoption option, const T& arg) const
  {
    string text = "curl_multi_setopt(" + std::to_string((int)option) + ")";
    check_(curl_multi_setopt(handle(), option, arg), text.c_str());
  }

protected:
  static void check_(CURLMcode err, const char* from);
  static handle_t check_(handle_t h);

private:
  static int action_(CURL* easy,
                     curl_socket_t s,
                     int what,
                     void* userp,
                     void* socketp);
  static int close_(void* userp, curl_socket_t s);
  static int options_(void* userp,
                      curl_socket_t s,
                      curlsocktype purpose);
  static int setTimer_(CURLM* multi,
                       long timeout_ms,
                       void* userp);
  static curl_socket_t socket_(void* userp,
                             curlsocktype purpose,
                             curl_sockaddr* address);

private:
  using mutex_t = boost::recursive_mutex;
  using guard_t = boost::unique_lock<mutex_t>;

private:
  string name_;
  handle_t handle_;
  mutex_t mutex_;
};

} } //namespace pe { namespace curl {

#endif //_PE_CURL_MULTI__H_20191120_