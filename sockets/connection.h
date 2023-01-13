#ifndef _PE_SOCKETS_CONNECTION__H_20200312_
#define _PE_SOCKETS_CONNECTION__H_20200312_

#include "iconnection.h"

#include <queue>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>

namespace pe { namespace sockets {

class BaseConnection : public IConnection
                     , public std::enable_shared_from_this<BaseConnection>
                     , noncopyable
{
public:
  BaseConnection(boost::asio::io_service& ios);
  BaseConnection(boost::asio::io_service& ios, const CID& id);
  virtual ~BaseConnection() override;

public:
  virtual const CID& id() const override { return id_; }
  virtual boost::asio::io_service& ios() override;
  virtual void recv(buffer& dst, RecvCallback&& callback) override;
  virtual void write(const MessagePtr& msg) override;

protected:
  virtual void onConnect();
  virtual void onConnectError(const boost::system::error_code& error);
  virtual void onRecvComplete(size_t nread, const string& addr, uint16_t port);
  virtual void onRecvError(const boost::system::error_code& error,
                           size_t nread,
                           const string& addr,
                           uint16_t port);
  virtual void onReadComplete(size_t nread);
  virtual void onReadError(const boost::system::error_code& error, size_t nread);
  virtual void onWriteComplete(size_t nwritten);
  virtual void onWriteError(const boost::system::error_code& error, size_t nwritten);
  virtual void send(const Message& msg, WriteCallback&& handler)=0;

private:
  void write();

private:
  static CID create_id();

private:
  CID id_;
  boost::asio::io_service::strand strand_;
  std::queue<MessagePtr> queue_;
};

} } //namespace pe { namespace sockets

#endif //#ifndef _PE_SOCKETS_CONNECTION__H_20200312_
