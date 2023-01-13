#include "connection.h"

#include "trace.h"

namespace pe { namespace sockets { namespace tcp {

PE_DECLARE_LOG_CHANNEL(CONNECTION_LOG, "tcp.connection[");
#define THISLOG CONNECTION_LOG << id() << "]: "

Connection::Connection(boost::asio::io_service& _ios,
                       const SocketPtr& _socket)
  : _Base(_ios)
  , socket_(_socket)
{
  PE_DEBUG(THISLOG << "created...");
}

Connection::Connection(boost::asio::io_service& _ios,
                       uint16_t _port,
                       const string& _addr)
  : Connection(_ios, create(_ios, _port, _addr))
{
  PE_DEBUG(THISLOG << "created and bound to interface '" << addr() << ":" << port() << "'...");
}

Connection::~Connection()
{
  PE_DEBUG(THISLOG << "destroying...");
}

Connection::SocketPtr Connection::create(boost::asio::io_service& ios,
                                         uint16_t port,
                                         const string& addr)
{
  auto s = std::make_shared<Socket>(ios);

  using namespace boost::asio;

  ip::tcp::endpoint endpoint(ip::address::from_string(addr), port);
  s->open(endpoint.protocol());
  s->set_option(ip::tcp::socket::reuse_address(true));
  s->bind(endpoint);

  return s;
}

const string& Connection::ipv4addr(const string& addr)
{
  static const string addrANY("0.0.0.0");
  return addr.empty() ? addrANY : addr;
}

boost::system::error_code Connection::shutdown(const boost::asio::socket_base::shutdown_type& how)
{
  boost::system::error_code ec;
  socket().shutdown(how, ec);
  return ec;
}

boost::system::error_code Connection::close()
{
  boost::system::error_code ec;
  socket().close(ec);
  return ec;
}

void Connection::connect(const string& addr, uint16_t port, ConnectCallback&& callback)
{
  auto self = shared_from_this();

  auto handler = [this, self, callback](const boost::system::error_code& error) {

    if (!error)
      onConnect();
    else
      onConnectError(error);

    try
    {
      if (callback)
        callback(error);
    }
    catch(const std::exception& e)
    {
      PE_ERROR(THISLOG << "exception in connect callback: " << e.what());
    }
  };

  using namespace boost::asio;
  ip::tcp::endpoint dest(ip::address::from_string(addr), port);

  socket().async_connect(dest, handler);
}

void Connection::send(const Message& msg, WriteCallback&& handler)
{
  socket().async_send(msg, handler);
}

void Connection::onWriteComplete(size_t nwritten)
{
  //PE_TRACE(THISLOG << "write complete, nwritten = " << nwritten);
  _Base::onWriteComplete(nwritten);
}

void Connection::onWriteError(const boost::system::error_code& error, size_t nwritten)
{
  PE_ERROR(THISLOG << "error writing to socket: " << error.message() << ", nwritten = " << nwritten);
  _Base::onWriteError(error, nwritten);
}

void Connection::read(buffer& dst, ReadCallback&& callback)
{
  auto self = shared_from_this();

  auto handler = [this, self, callback](const boost::system::error_code& error, size_t nread) {

    if (!error)
      onReadComplete(nread);
    else
      onReadError(error, nread);

    try
    {
      if (callback)
        callback(error, nread);
    }
    catch(const std::exception& e)
    {
      PE_ERROR(THISLOG << "exception in read callback: " << e.what());
    }
  };

  socket().async_read_some(boost::asio::buffer(dst), handler);
}

void Connection::onReadComplete(size_t nread)
{
  //PE_TRACE(THISLOG << "read complete, nread = " << nread);
  _Base::onReadComplete(nread);
}

void Connection::onReadError(const boost::system::error_code& error, size_t nread)
{
  if (error == boost::asio::error::eof)
  {
    PE_DEBUG(THISLOG << "EOF on socket... connection will be closed");
  }
  else if (error == boost::asio::error::operation_aborted)
  {
    PE_DEBUG(THISLOG << "operation canceled");
  }
  else
  {
    PE_ERROR(THISLOG << "error reading from socket: " << error.message() << ", nread = " << nread);
  }

  _Base::onReadError(error, nread);
}

} } } //namespace pe { namespace sockets { namespace tcp {
