#ifndef _PE_SOCKETS_WEB_CONNECTION__H_20190313_
#define _PE_SOCKETS_WEB_CONNECTION__H_20190313_

#include "wsdefs.h"

#include "sockets/connection.h"
#include "mutex.h"

namespace pe { namespace sockets { namespace web {

class Connection : public BaseConnection
{
public:
  using _Base = BaseConnection;
  using SocketPtr = ConnectionPtr;

public:
  Connection(boost::asio::io_service& ios,
             const SocketPtr& socket);
  virtual ~Connection() override;

public:
  virtual string addr() const override { return socket_->get_host(); }
  virtual uint16_t port() const override { return socket_->get_port(); }
  virtual string remoteAddr() const override;
  virtual uint16_t remotePort() const override;
  virtual boost::system::error_code close() override;
  virtual void connect(const string& addr, uint16_t port, ConnectCallback&& callback) override;
  virtual void read(buffer& dst, ReadCallback&& callback) override;

protected:
  virtual void onReadComplete(size_t nread) override;
  virtual void onReadError(const boost::system::error_code& error, size_t nread) override;
  virtual void onWriteComplete(size_t nwritten) override;
  virtual void onWriteError(const boost::system::error_code& error, size_t nwritten) override;
  virtual void send(const Message& msg, WriteCallback&& handler) override;

private:
  void read(ReadCallback&& handler);
  ReadCallback resetHandlers();
  void setHandlers();

private:
  SocketPtr socket_;
  ReadCallback onread_;
  mutex_t mtxhandlers_;
  bool handlersSet_ = false;
};

} } } //namespace pe { namespace sockets { namespace web {

#endif //#ifndef _PE_SOCKETS_WEB_CONNECTION__H_20190313_
