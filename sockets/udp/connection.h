#ifndef _PE_SOCKETS_UDP_CONNECTION__H_20200712_
#define _PE_SOCKETS_UDP_CONNECTION__H_20200712_

#include "sockets/connection.h"

#include <boost/asio/ip/udp.hpp>

namespace pe { namespace sockets { namespace udp {

class Connection : public BaseConnection
{
  using _Base = BaseConnection;

public:
  using Socket = boost::asio::ip::udp::socket;

public:
  Connection(boost::asio::io_service& ios,
             uint16_t port,
             const string& addr=string(),
             uint8_t children=1);
  virtual ~Connection() override;

public:
  virtual string addr() const override { return socket().local_endpoint().address().to_string(); }
  virtual uint16_t port() const override { return socket().local_endpoint().port(); }
  virtual string remoteAddr() const override { return socket().remote_endpoint().address().to_string(); }
  virtual uint16_t remotePort() const override { return socket().remote_endpoint().port(); }
  virtual boost::system::error_code close() override;
  virtual void connect(const string& addr, uint16_t port, ConnectCallback&& callback) override;
  virtual void read(buffer& dst, ReadCallback&& callback) override;
  virtual void recv(buffer& dst, RecvCallback&& callback) override;

protected:
  Socket& socket() { return socket_; }
  const Socket& socket() const { return socket_; }
  uint8_t& children() { return children_; }
  const uint8_t& children() const { return children_; }

protected:
  virtual void onRecvComplete(size_t nread, const string& addr, uint16_t port) override;
  virtual void onRecvError(const boost::system::error_code& error,
                           size_t nread,
                           const string& addr,
                           uint16_t port) override;
  virtual void onReadComplete(size_t nread) override;
  virtual void onReadError(const boost::system::error_code& error, size_t nread) override;
  virtual void onWriteComplete(size_t nwritten) override;
  virtual void onWriteError(const boost::system::error_code& error, size_t nwritten) override;
  virtual void send(const Message& msg, WriteCallback&& handler) override;

protected:
  void bind(const string& addr, uint16_t port);

private:
  static const string& ipv4addr(const string& addr);
  static CID getCID(uint16_t port) { return port; }

private:
  using Endpoint = Socket::endpoint_type;

private:
  Socket socket_;
  Endpoint recvremote_;
  uint8_t children_;
};

} } } //namespace pe { namespace sockets { namespace udp {

#endif //#ifndef _PE_SOCKETS_UDP_CONNECTION__H_20200712_
