#ifndef _PE_SOCKETS_TCP_SERVER__H_20191002_
#define _PE_SOCKETS_TCP_SERVER__H_20191002_

#include "sockets/defs.h"

#include <functional>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace pe { namespace sockets { namespace tcp {

class Server : public std::enable_shared_from_this<Server>
             , noncopyable
{
public:
  using Socket = boost::asio::ip::tcp::socket;
  using SocketPtr = std::shared_ptr<Socket>;
  using AcceptCallback = std::function<bool(const SocketPtr&)>;

public:
  Server(boost::asio::io_service& ios,
         uint16_t port,
         const string& addr=string(),
         int backlog = boost::asio::socket_base::max_connections);
  Server(boost::asio::io_service& ios,
         uint16_t minport,
         uint16_t maxport,
         const string& addr=string(),
         int backlog = boost::asio::socket_base::max_connections);
  virtual ~Server();

public:
  void accept(AcceptCallback&& acceptcb);
  void cancel();

public:
  string addr() const { return listener_.local_endpoint().address().to_string(); }
  uint16_t port() const { return listener_.local_endpoint().port(); }
  uint64_t id() const { return port(); }
  boost::asio::io_service& ios() { return ios_; }

private:
  void listen(boost::asio::ip::tcp::acceptor& acceptor,
              const string& addr,
              uint16_t port,
              int backlog);
  void listen(boost::asio::ip::tcp::acceptor& acceptor,
              const string& addr,
              uint16_t minport,
              uint16_t maxport,
              int backlog);

private:
  static const string& ipv4addr(const string& addr);
  static uint16_t port(uint16_t rmax, uint16_t rmin=0);

private:
  boost::asio::io_service& ios_;
  boost::asio::ip::tcp::acceptor listener_;
};

} } } //namespace pe { namespace sockets { namespace tcp {

#endif //#ifndef _PE_SOCKETS_TCP_SERVER__H_20191002_
