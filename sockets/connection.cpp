#include "connection.h"

#include <atomic>

namespace pe { namespace sockets {

BaseConnection::BaseConnection(boost::asio::io_service& ios)
  : BaseConnection(ios, create_id())
{
}

BaseConnection::BaseConnection(boost::asio::io_service& ios, const CID& id)
  : id_(id)
  , strand_(ios)
{
}

BaseConnection::~BaseConnection()
{
}

BaseConnection::CID BaseConnection::create_id()
{
  static std::atomic<CID> __id__{ 1000000 };
  return ++__id__;
}

boost::asio::io_service& BaseConnection::ios()
{
#if defined(BOOST_VERSION) && (BOOST_VERSION >= 107000)
  return strand_.context();
#else
  return strand_.get_io_service();
#endif
}

void BaseConnection::recv(buffer& dst, RecvCallback&& callback)
{
  PE_UNUSED(dst);
  PE_UNUSED(callback);
  throw exceptions::not_implemented("recv() not implemented for this class");
}

void BaseConnection::write(const MessagePtr& msg)
{
  auto self = shared_from_this();

  strand_.post([this, self, msg]() {

    queue_.push(msg);

    if (queue_.size()>1)
      return;

    write();
  });
}

void BaseConnection::write()
{
  auto self = shared_from_this();

  auto handler = [this, self](const boost::system::error_code& error, size_t nwritten) {

    if (!error)
    {
      queue_.pop();

      onWriteComplete(nwritten);

      if (!queue_.empty())
        write();
    }
    else
    {
      onWriteError(error, nwritten);
    }
  };

  send(*queue_.front(), strand_.wrap(handler));
}

void BaseConnection::onConnect()
{
}

void BaseConnection::onConnectError(const boost::system::error_code& error)
{
  PE_UNUSED(error);
}

void BaseConnection::onRecvComplete(size_t nread, const string& addr, uint16_t port)
{
  PE_UNUSED(nread);
  PE_UNUSED(addr);
  PE_UNUSED(port);
}

void BaseConnection::onRecvError(const boost::system::error_code& error,
                                 size_t nread,
                                 const string& addr,
                                 uint16_t port)
{
  PE_UNUSED(error);
  PE_UNUSED(nread);
  PE_UNUSED(addr);
  PE_UNUSED(port);
}

void BaseConnection::onReadComplete(size_t nread)
{
  PE_UNUSED(nread);
}

void BaseConnection::onReadError(const boost::system::error_code& error, size_t nread)
{
  PE_UNUSED(error);
  PE_UNUSED(nread);
}

void BaseConnection::onWriteComplete(size_t nwritten)
{
  PE_UNUSED(nwritten);
}

void BaseConnection::onWriteError(const boost::system::error_code& error, size_t nwritten)
{
  PE_UNUSED(error);
  PE_UNUSED(nwritten);
}

} } //namespace pe { namespace sockets {