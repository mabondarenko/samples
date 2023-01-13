#ifndef _PE_SOCKETS_TCP_CONNECTION__H_20191002_
#define _PE_SOCKETS_TCP_CONNECTION__H_20191002_

#include "sockets/connection.h"

#include <boost/asio/ip/tcp.hpp>

namespace pe { namespace sockets { namespace tcp {

class Connection : public BaseConnection
{
public:
  using _Base = BaseConnection;
  using Socket = boost::asio::ip::tcp::socket;
  using SocketPtr = std::shared_ptr<Socket>;

public:
  Connection(boost::asio::io_service& ios,
             const SocketPtr& socket);
  Connection(boost::asio::io_service& ios,
             uint16_t port,
             const string& addr=string());
  virtual ~Connection() override;

public:
  virtual string addr() const override { return socket().local_endpoint().address().to_string(); }
  virtual uint16_t port() const override { return socket().local_endpoint().port(); }
  virtual string remoteAddr() const override { return socket().remote_endpoint().address().to_string(); }
  virtual uint16_t remotePort() const override { return socket().remote_endpoint().port(); }
  virtual boost::system::error_code close() override;
  virtual void connect(const string& addr, uint16_t port, ConnectCallback&& callback) override;
  virtual void read(buffer& dst, ReadCallback&& callback) override;

public:
  boost::system::error_code shutdown(const boost::asio::socket_base::shutdown_type& how=Socket::shutdown_both);

protected:
  Socket& socket() { return *socket_; }
  const Socket& socket() const { return *socket_; }

protected:
  virtual void onReadComplete(size_t nread) override;
  virtual void onReadError(const boost::system::error_code& error, size_t nread) override;
  virtual void onWriteComplete(size_t nwritten) override;
  virtual void onWriteError(const boost::system::error_code& error, size_t nwritten) override;
  virtual void send(const Message& msg, WriteCallback&& handler) override;

private:
  static const string& ipv4addr(const string& addr);
  static SocketPtr create(boost::asio::io_service& ios,
                          uint16_t port,
                          const string& addr=string());

private:
  SocketPtr socket_;
};

} } } //namespace pe { namespace sockets { namespace tcp {

#endif //#ifndef _PE_SOCKETS_TCP_CONNECTION__H_20191002_
