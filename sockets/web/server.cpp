#include "server.h"

#include <boost/random.hpp>

#include "trace.h"

namespace pe { namespace sockets { namespace web {

PE_DECLARE_LOG_CHANNEL(SRV_LOG, "websocket.server[");
#define THISLOG SRV_LOG << id() << "]: "

Server::Server(boost::asio::io_service& ios,
               uint16_t _port,
               const string& _addr,
               int backlog)
{
  init(ios, backlog);

  const auto& addr = ipv4addr(_addr);
  listen(addr, _port);

  PE_DEBUG(THISLOG << "created and bound to interface '" << addr << ":" << port() << "'...");
}

Server::Server(boost::asio::io_service& ios,
               uint16_t minport,
               uint16_t maxport,
               const string& _addr,
               int backlog)
{
  init(ios, backlog);

  const auto& addr = ipv4addr(_addr);
  listen(addr, minport, maxport);

  PE_DEBUG(THISLOG << "created and bound to interface '" << addr << ":" << port() << "'...");
}

Server::~Server()
{
  PE_DEBUG(THISLOG << "destroying...");
}

void Server::init(boost::asio::io_service& ios, int backlog)
{
  listener_.init_asio(&ios);

  listener_.set_listen_backlog(backlog);
  listener_.set_reuse_addr(true);

#ifndef _DEBUG
  listener_.set_access_channels(websocketpp::log::alevel::none);
  listener_.set_error_channels(websocketpp::log::alevel::none);
#else
  //listener_.set_access_channels(websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);
  listener_.set_access_channels(websocketpp::log::alevel::all);
  listener_.clear_access_channels(websocketpp::log::alevel::frame_payload);
  listener_.clear_access_channels(websocketpp::log::alevel::frame_header);
  listener_.set_error_channels(websocketpp::log::alevel::all);
#endif
}

const string& Server::ipv4addr(const string& addr)
{
  static const string addrANY("0.0.0.0");
  return addr.empty() ? addrANY : addr;
}

void Server::cancel()
{
  PE_DEBUG(THISLOG << "canceling...");

  listener_.stop_listening();
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
  auto s = listener_.get_connection();

  if (!s)
    throw exceptions::runtime_error("failed to create websocket connection");

  auto self = shared_from_this();
  listener_.async_accept(s, [this, s, self, acceptcb](const websocketpp::lib::error_code& error) {
    if (!error)
    {
      try
      {
        if (!acceptcb(s))
          s->terminate(error);
      }
      catch(const std::exception& e)
      {
        PE_ERROR(THISLOG << "exception in accept callback: " << e.what());

        s->terminate(error);
      }
    }
    else
    {
      s->terminate(error);

      if (error == websocketpp::error::operation_canceled)
      {
        PE_DEBUG(THISLOG << "listen operation canceled");
      }
      else
      {
        PE_ERROR(THISLOG << "error while accepting a socket: " << error.message());
      }
    }
  });
}

void Server::listen(const string& addr, uint16_t port)
{
  using namespace boost::asio;

  ip::tcp::endpoint endpoint(ip::address::from_string(addr), port);
  listener_.listen(endpoint);
  port_ = port;
}

void Server::listen(const string& addr, uint16_t minport, uint16_t maxport)
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
      return listen(addr, p);
    }
    catch(const std::exception& e)
    {
      PE_ERROR(THISLOG << "failed to start listen at '" << addr << ":" << p << "': " << e.what());

      if (!ntry)
        throw; //re-throw current exception
    }
  };
}

} } } //namespace pe { namespace sockets { namespace web {
