#ifndef _PE_SOCKETS_WEB_SERVER__H_20200312_
#define _PE_SOCKETS_WEB_SERVER__H_20200312_

#include "wsdefs.h"

#include <functional>

#include <boost/asio/io_service.hpp>

namespace pe { namespace sockets { namespace web {

class Server : public std::enable_shared_from_this<Server>
             , noncopyable
{
public:
  using SocketPtr = ConnectionPtr;
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
  uint16_t port() const { return port_; }
  uint64_t id() const { return port(); }
  boost::asio::io_service& ios() { return listener_.get_io_service(); }

private:
  void init(boost::asio::io_service& ios, int backlog);
  void listen(const string& addr,
              uint16_t port);
  void listen(const string& addr,
              uint16_t minport,
              uint16_t maxport);

private:
  static const string& ipv4addr(const string& addr);
  static uint16_t port(uint16_t rmax, uint16_t rmin=0);

private:
  Acceptor listener_;
  uint16_t port_ = 0;
};

} } } //namespace pe { namespace sockets { namespace web {

#endif //#ifndef _PE_SOCKETS_WEB_SERVER__H_20200312_
