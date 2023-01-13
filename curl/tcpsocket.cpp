#include "tcpsocket.h"

#include "trace.h"

namespace pe { namespace curl {

PE_DECLARE_LOG_CHANNEL(SOCKLOG, "curl.tcp.socket");
#define THISLOG SOCKLOG << "[" << handle() << "]: "

#define TCP_BOOST_PROTOCOL(family) ((family)==AF_INET6 ? boost::asio::ip::tcp::v6() : boost::asio::ip::tcp::v4())

TcpSocket::TcpSocket(boost::asio::io_service& _ios, int _family)
  : socket_(_ios)
{
  socket_.open(TCP_BOOST_PROTOCOL(_family));
  socket_.non_blocking(true);

  PE_DEBUG(THISLOG << "created...");
}

TcpSocket::~TcpSocket()
{
  PE_DEBUG(THISLOG << "destroying...");
}

void TcpSocket::read(handler_t&& h, int mask)
{
  auto self = shared_from_this();
  auto handler = [self, h, mask, this](const error_t& error, std::size_t) {
    if (h && error!=boost::asio::error::operation_aborted)
      h(this, error, mask);
  };

  socket_.async_read_some(boost::asio::null_buffers(), handler);
}

void TcpSocket::write(handler_t&& h, int mask)
{
  auto self = shared_from_this();
  auto handler = [self, h, mask, this](const error_t& error, std::size_t) {
    if (h && error!=boost::asio::error::operation_aborted)
      h(this, error, mask);
  };

  socket_.async_write_some(boost::asio::null_buffers(), handler);
}

} } //namespace pe { namespace curl {