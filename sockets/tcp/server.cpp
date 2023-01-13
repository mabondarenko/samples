#include "server.h"

#include <boost/random.hpp>

#include "trace.h"

namespace pe { namespace sockets { namespace tcp {

PE_DECLARE_LOG_CHANNEL(SRV_LOG, "tcp.server[");
#define THISLOG SRV_LOG << id() << "]: "

Server::Server(boost::asio::io_service& ios,
               uint16_t _port,
               const string& _addr,
               int backlog)
  : ios_(ios)
  , listener_(ios)
{
  listen(listener_, ipv4addr(_addr), _port, backlog);

  PE_DEBUG(THISLOG << "created and bound to interface '" << addr() << ":" << port() << "'...");
}

Server::Server(boost::asio::io_service& ios,
               uint16_t minport,
               uint16_t maxport,
               const string& _addr,
               int backlog)
  : ios_(ios)
  , listener_(ios)
{
  listen(listener_, ipv4addr(_addr), minport, maxport, backlog);

  PE_DEBUG(THISLOG << "created and bound to interface '" << addr() << ":" << port() << "'...");
}

Server::~Server()
{
  PE_DEBUG(THISLOG << "destroying...");
}

const string& Server::ipv4addr(const string& addr)
{
  static const string addrANY("0.0.0.0");
  return addr.empty() ? addrANY : addr;
}

void Server::cancel()
{
  PE_DEBUG(THISLOG << "canceling...");

  listener_.cancel();
}

uint16_t Server::port(uint16_t rmax, uint16_t rmin)
{
  if (rmin==rmax)
    return rmax;

  static boost::random::mt19937 gen{static_cast<uint32_t>(std::time(nullptr))};
  boost::random::uniform_int_distribution<uint16_t> dist{rmin, rmax};
  return dist(gen);
}

void Server::accept(AcceptCallback&& acceptcb)
{
  SocketPtr s(new Socket(ios()));

  auto self = shared_from_this();
  listener_.async_accept(*s.get(), [this, s, self, acceptcb](const boost::system::error_code& error) {
    if (!error)
    {
      try
      {
        acceptcb(s);
      }
      catch(const std::exception& e)
      {
        PE_ERROR(THISLOG << "exception in accept callback: " << e.what());
      }
    }
    else if (error == boost::asio::error::operation_aborted)
    {
      PE_DEBUG(THISLOG << "listen operation canceled");
    }
    else
    {
      PE_ERROR(THISLOG << "error while accepting a socket: " << error.message());
    }
  });
}

void Server::listen(boost::asio::ip::tcp::acceptor& acceptor,
                    const string& addr,
                    uint16_t port,
                    int backlog)
{
  using namespace boost::asio;

  ip::tcp::endpoint endpoint(ip::address::from_string(addr), port);
  acceptor.open(endpoint.protocol());
  acceptor.set_option(ip::tcp::acceptor::reuse_address(true));
  acceptor.bind(endpoint);
  acceptor.listen(backlog);
}

void Server::listen(boost::asio::ip::tcp::acceptor& acceptor,
                    const string& addr,
                    uint16_t minport,
                    uint16_t maxport,
                    int backlog)
{
  if (minport>maxport)
  {
    if (maxport>0)
      std::swap(minport, maxport);
    else
      maxport = minport;
  }

  size_t ntry = (maxport - minport) + 1;

  if (ntry > 5000)
    ntry = 5000;

  while(ntry-- > 0)
  {
    uint16_t p = port(maxport, minport);

    try
    {
      return listen(acceptor, addr, p, backlog);
    }
    catch(const std::exception& e)
    {
      PE_ERROR(THISLOG << "failed to start listen at '" << addr << ":" << p << "': " << e.what());

      if (!ntry)
        throw; //re-throw current exception
    }
  };
}

} } } //namespace pe { namespace sockets { namespace tcp {
