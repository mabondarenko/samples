#include "service.h"

#include "easy.h"
#include "trace.h"
#include "transfer.h"
#include "tcpsocket.h"
#include "udpsocket.h"

namespace pe { namespace curl {

PE_DECLARE_LOG_CHANNEL(SRVLOG, "curl.service");
#define THISLOG SRVLOG << "[" << name() << "]: "

Service::Service(boost::asio::io_service& _ios,
                 const string& _name)
  : _Base(_name)
  , ios_(_ios)
  , timer_(ios())
  , pinger_(ios())
{
  PE_DEBUG(THISLOG << "created...");
  PE_INFO(THISLOG << "version: " << curl_version());
}

Service::~Service()
{
  PE_DEBUG(THISLOG << "destroying...");

  stop();

  destroy();
}

void Service::stop()
{
  PE_DEBUG(THISLOG << "stopping...");

  pinger_.cancel();
}

void Service::pinger()
{
  pinger_.expires_from_now(boost::posix_time::seconds(60));
  pinger_.async_wait([this](const boost::system::error_code& error) {
    if (!error)
      pinger();
  });
}

int Service::setTimer(long timeout_ms)
{
  if (!pingerset_)
  {
    pinger();
    pingerset_ = true;
  }

  cancelTimer();

  if (timeout_ms>0)
  {
    auto self = shared_from_this();
    timer_.expires_from_now(boost::posix_time::millisec(timeout_ms));
    timer_.async_wait([this, self](const boost::system::error_code& error) {
      if (!error)
        timer();
    });
  }
  else if (timeout_ms==0)
  {
    //hold self while calling timer
    auto self = shared_from_this();
    timer();
  }

  return 0;
}

int Service::action(CURL* easy,
                    curl_socket_t s,
                    int what,
                    void* socketp)
{
  PE_UNUSED(easy);

#ifdef _CURL_DEBUG
  PE_TRACE(THISLOG << "action_callback() begin: socket=" << s);
#endif

  auto sock = reinterpret_cast<Socket*>(socketp);

  switch(what)
  {
  case CURL_POLL_IN:
  case CURL_POLL_OUT:
  case CURL_POLL_INOUT:
    if (!sock)
      add(s, what);
    else if (get(s))
      update(sock, what, sock->mask());
    break;

  case CURL_POLL_REMOVE:
    if (sock && get(s) && sock->mask())
    {
#ifdef _CURL_DEBUG
      PE_TRACE(THISLOG << "CURL_POLL_REMOVE: socket=" << sock->handle());
#endif
      sock->mask(0);
      sock->cancel();
    }
    break;

  default:
    PE_ERROR(THISLOG << "unknown socket action " << what << " for socket=" << s);
    break;
  }

#ifdef _CURL_DEBUG
  PE_TRACE(THISLOG << "action_callback() end: socket=" << s);
#endif

  return 0;
}

void Service::add(curl_socket_t s, int what)
{
  auto sock = get(s);
  if (!sock)
    return;

#ifdef _CURL_DEBUG
  PE_TRACE(THISLOG << "add socket=" << s << ", what=" << what);
#endif

  assign(s, sock.get());
  update(sock.get(), what, 0);
}

void Service::update(Socket* sock, int what, int prev)
{
#ifdef _CURL_DEBUG
  PE_TRACE(THISLOG << "update() begin: socket=" << sock->handle() << ", what=" << what << ", prev=" << prev);
#endif

  auto self = shared_from_this();
  auto handler = [this, self](Socket* sock,
                              const boost::system::error_code& error,
                              int mask)
  {
#ifdef _CURL_DEBUG
    PE_TRACE(THISLOG << "socket_handler begin: socket=" << sock->handle() << ", mask=" << mask);
#endif

    if (error)
    {
      mask |= CURL_CSELECT_ERR;
      PE_ERROR(THISLOG << "socket=" << sock->handle() << ", error: " << error.message());
    }

    //store current event mask before process() during it mask may be changed
    auto prev = sock->mask();

#ifdef _CURL_DEBUG
    if (mask&CURL_POLL_REMOVE)
    {
      PE_ERROR(THISLOG << "socket_handler CURL_POLL_REMOVE for socket=" << sock->handle());
    }
#endif

    process(sock->handle(), mask);

    //continue monitoring if:
    //  1. no errors and
    //  2. socket is not closed
    //  3. no CURL_POLL_REMOVE in saction
    if (!error && get(sock->handle()) && sock->mask())
      update(sock, sock->mask(), prev & ~mask);

#ifdef _CURL_DEBUG
    PE_TRACE(THISLOG << "socket_handler end: socket=" << sock->handle() << ", mask=" << mask);
#endif
  };

  sock->mask(what);

  if ((what&CURL_POLL_IN)==CURL_POLL_IN && (prev&CURL_POLL_IN)==0)
    sock->read(handler, CURL_CSELECT_IN);

  if ((what&CURL_POLL_OUT)==CURL_POLL_OUT && (prev&CURL_POLL_OUT)==0)
    sock->write(handler, CURL_CSELECT_OUT);

#ifdef _CURL_DEBUG
  PE_TRACE(THISLOG << "update() end: socket=" << sock->handle() << ", what=" << what << ", prev=" << prev);
#endif
}

Service::SocketPtr Service::get(curl_socket_t s)
{
  auto it = sockets_.find(s);
  if (it!=sockets_.end())
    return it->second;
  return SocketPtr();
}

bool Service::remove(curl_socket_t s)
{
#ifdef _CURL_DEBUG
  PE_TRACE(THISLOG << "remove socket=" << s);
#endif
  return sockets_.erase(s)>0;
}

curl_socket_t Service::socket(curlsocktype purpose, curl_sockaddr* address)
{
  if ((purpose==CURLSOCKTYPE_IPCXN || purpose==CURLSOCKTYPE_ACCEPT) && address)
  {
    try
    {
      SocketPtr s;

      if (address->socktype==SOCK_DGRAM)
        s.reset(new UdpSocket(ios(), address->family));
      else if (address->socktype==SOCK_STREAM)
        s.reset(new TcpSocket(ios(), address->family));

      if (s)
      {
        sockets_[s->handle()] = s;
        return s->handle();
      }
    }
    catch(const std::exception& e)
    {
      PE_ERROR(THISLOG << "failed to create socket for transfer: " << e.what());
    }
  }

  return CURL_SOCKET_BAD;
}

int Service::options(curl_socket_t s, curlsocktype purpose)
{
  auto sock = get(s);
  if (!sock)
    return CURL_SOCKOPT_ERROR;

  PE_UNUSED(purpose);

  return CURL_SOCKOPT_OK;
}

int Service::close(curl_socket_t s)
{
  return remove(s) ? 0 : 1;
}

Service::TransferPtr Service::transfer(const string& url)
{
  return std::make_shared<Transfer>(shared_from_this(), url);
}

} } //namespace pe { namespace curl {
