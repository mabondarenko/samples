#ifndef _PE_CURL_SERVICE__H_20190917_
#define _PE_CURL_SERVICE__H_20190917_

#include "types.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>

#include "multi.h"

namespace pe { namespace curl {

class Socket;
class Transfer;

class Service : public Multi
              , public std::enable_shared_from_this<Service>
{
  using _Base = Multi;
  using SocketPtr = std::shared_ptr<Socket>;
  using TransferPtr = std::shared_ptr<Transfer>;

public:
  Service(boost::asio::io_service& ios, const string& name);
  virtual ~Service();

public:
  boost::asio::io_service& ios() { return ios_; }
  TransferPtr transfer(const string& url);
  void stop();

protected:
  virtual int action(CURL* easy,
                     curl_socket_t s,
                     int what,
                     void* socketp) override;
  virtual void cancelTimer() override { timer_.cancel(); }
  virtual int close(curl_socket_t s) override;
  virtual int options(curl_socket_t s, curlsocktype purpose) override;
  virtual int setTimer(long timeout_ms) override;
  virtual curl_socket_t socket(curlsocktype purpose, curl_sockaddr* address) override;

protected:
  void add(curl_socket_t s, int what);
  SocketPtr get(curl_socket_t s);
  bool remove(curl_socket_t s);
  void update(Socket* sock, int what, int prev);
  void pinger();

private:
  using timer_t = boost::asio::deadline_timer;
  using socket_map_t = std::unordered_map<curl_socket_t, SocketPtr>;

private:
  boost::asio::io_service& ios_;
  timer_t timer_;
  timer_t pinger_;
  socket_map_t sockets_;
  bool pingerset_ = false;
};

} } //namespace pe { namespace curl {

#endif //#ifndef _PE_CURL_SERVICE__H_20190917_