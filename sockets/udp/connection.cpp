#include <linux/filter.h>
#include "connection.h"

#include "trace.h"

namespace pe { namespace sockets { namespace udp {

PE_DECLARE_LOG_CHANNEL(CONNECTION_LOG, "udp.connection[");
#define THISLOG CONNECTION_LOG << id() << "]: "
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

Connection::Connection(boost::asio::io_service& _ios,
                       uint16_t _port,
                       const string& _addr,
                       uint8_t _children)
  : _Base(_ios)
  , socket_(_ios)
  , children_(_children)
{
  bind(ipv4addr(_addr), _port);

  PE_DEBUG(THISLOG << "created and bound to interface '" << addr() << ":" << port() << "'...");
}

Connection::~Connection()
{
  PE_DEBUG(THISLOG << "destroying...");
}

const string& Connection::ipv4addr(const string& addr)
{
  static const string addrANY("0.0.0.0");
  return addr.empty() ? addrANY : addr;
}

void Connection::bind(const string& addr, uint16_t port)
{
  using namespace boost::asio;

  ip::udp::endpoint endpoint(ip::address::from_string(addr), port);
  socket().open(endpoint.protocol());
  socket().set_option(ip::udp::socket::reuse_address(true));
  typedef boost::asio::detail::socket_option::boolean<SOL_SOCKET, SO_REUSEPORT> reuse_port;
  socket().set_option(reuse_port(true));

  struct sock_filter code[] = {
    /* A = (uint32_t)skb[0] */
    { BPF_LD  | BPF_W | BPF_ABS, 0, 0, static_cast<unsigned int>(SKF_AD_OFF + SKF_AD_RANDOM) },
    /* A = A % mod */
    { BPF_ALU | BPF_MOD, 0, 0, children() },
    /* return A */
    { BPF_RET | BPF_A, 0, 0, 0 }
  };

  struct sock_fprog fprog = {
    .len = ARRAY_SIZE(code),
    .filter = code,
  };

  socket().non_blocking(true);
  socket().bind(endpoint);

  if (children() > 1)
  {
    PE_DEBUG(THISLOG << "enable random distribution on " << std::to_string(children()) << " ports");
    setsockopt(socket().native_handle(), SOL_SOCKET, SO_ATTACH_REUSEPORT_CBPF, &fprog, sizeof(fprog));
  }
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
  ip::udp::endpoint dest(ip::address::from_string(addr), port);

  socket().async_connect(dest, handler);
}

void Connection::send(const Message& msg, WriteCallback&& handler)
{
  if (msg.addr().empty() && !msg.port())
    socket().async_send(msg, handler);
  else
  {
    using namespace boost::asio;
    ip::udp::endpoint dest(ip::address::from_string(msg.addr()), msg.port());
    socket().async_send_to(msg, dest, handler);
  }
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

  socket().async_receive(boost::asio::buffer(dst), handler);
}

void Connection::recv(buffer& dst, RecvCallback&& callback)
{
  auto self = shared_from_this();

  auto handler = [this, self, callback](const boost::system::error_code& error, size_t nread) {

    if (!error)
      onRecvComplete(nread, recvremote_.address().to_string(), recvremote_.port());
    else
      onRecvError(error, nread, recvremote_.address().to_string(), recvremote_.port());

    try
    {
      if (callback)
        callback(error, nread, recvremote_.address().to_string(), recvremote_.port());
    }
    catch(const std::exception& e)
    {
      PE_ERROR(THISLOG << "exception in recv callback: " << e.what());
    }
  };

  socket().async_receive_from(boost::asio::buffer(dst), recvremote_, handler);
}

void Connection::onRecvComplete(size_t nread, const string& addr, uint16_t port)
{
  _Base::onRecvComplete(nread, addr, port);
}

void Connection::onRecvError(const boost::system::error_code& error,
                             size_t nread,
                             const string& addr,
                             uint16_t port)
{
  if (error == boost::asio::error::eof)
  {
    PE_DEBUG(THISLOG << "EOF on socket...");
  }
  else if (error == boost::asio::error::operation_aborted)
  {
    PE_DEBUG(THISLOG << "operation canceled");
  }
  else
  {
    PE_ERROR(THISLOG << "error recv from socket: " << error.message()
                     << ", addr='" << addr << "', port=" << port << ", nread=" << nread);
  }

  _Base::onRecvError(error, nread, addr, port);
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

} } } //namespace pe { namespace sockets { namespace udp {
