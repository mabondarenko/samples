#ifndef _PE_CURL_SOCKET__H_20190917_
#define _PE_CURL_SOCKET__H_20190917_

#include "types.h"

#include <boost/asio/error.hpp>

namespace pe { namespace curl {

class Socket
{
public:
  using error_t = boost::system::error_code;
  using handle_t = int;
  using handler_t = std::function<void(Socket* s,
                                       const error_t& error,
                                       int bitmask)>;

public:
  virtual ~Socket()=default;

public:
  virtual void cancel()=0;
  virtual handle_t handle() const=0;
  int mask() const { return mask_; }
  void mask(int m) { mask_ = m; }
  virtual void read(handler_t&& handler, int bitmask) = 0;
  virtual void write(handler_t&& handler, int bitmask) = 0;

private:
  int mask_ = 0;
};

} } //namespace pe { namespace curl {

#endif //#ifndef _PE_CURL_SOCKET__H_20190917_