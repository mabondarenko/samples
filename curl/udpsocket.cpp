#include "udpsocket.h"

#include "trace.h"

namespace pe { namespace curl {

PE_DECLARE_LOG_CHANNEL(SOCKLOG, "curl.udp.socket");
#define THISLOG SOCKLOG << "[" << handle() << "]: "

#define UDP_BOOST_PROTOCOL(family) ((family)==AF_INET6 ? boost::asio::ip::udp::v6() : boost::asio::ip::udp::v4())

UdpSocket::UdpSocket(boost::asio::io_service& _ios, int _family)
  : socket_(_ios)
{
  socket_.open(UDP_BOOST_PROTOCOL(_family));
  socket_.non_blocking(true);

  PE_DEBUG(THISLOG << "created...");
}

UdpSocket::~UdpSocket()
{
  PE_DEBUG(THISLOG << "destroying...");
}

void UdpSocket::read(handler_t&& h, int mask)
{
  auto self = shared_from_this();
  auto handler = [self, h, mask, this](const error_t& error, std::size_t) {
    if (h && error!=boost::asio::error::operation_aborted)
      h(this, error, mask);
  };

  socket_.async_receive(boost::asio::null_buffers(), handler);
}

void UdpSocket::write(handler_t&& h, int mask)
{
  auto self = shared_from_this();
  auto handler = [self, h, mask, this](const error_t& error, std::size_t) {
    if (h && error!=boost::asio::error::operation_aborted)
      h(this, error, mask);
  };

  socket_.async_send(boost::asio::null_buffers(), handler);
}

} } //namespace pe { namespace curl {