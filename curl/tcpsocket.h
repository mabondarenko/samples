#ifndef _PE_CURL_TCPSOCKET__H_20190918_
#define _PE_CURL_TCPSOCKET__H_20190918_

#include "socket.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace pe { namespace curl {

class TcpSocket : public std::enable_shared_from_this<TcpSocket>
                , public Socket
                , noncopyable
{
public:
  TcpSocket(boost::asio::io_service& ios, int family);
  virtual ~TcpSocket() override;

public:
  virtual void cancel() override { socket_.cancel(); }
  virtual handle_t handle() const override { return socket_.native_handle(); }
  virtual void read(handler_t&& handler, int bitmask) override;
  virtual void write(handler_t&& handler, int bitmask) override;

private:
  mutable boost::asio::ip::tcp::socket socket_;
};

} } //namespace pe { namespace curl {

#endif //#ifndef _PE_CURL_TCPSOCKET__H_20190918_