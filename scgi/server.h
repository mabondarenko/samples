#ifndef _PE_SCGI_SERVER__20190228_
#define _PE_SCGI_SERVER__20190228_

#include "types.h"

#include <boost/asio/io_service.hpp>

//forwards:
namespace pe { namespace sockets { namespace tcp {
  class Server;
} } } //namespace pe { namespace sockets { namespace tcp {

namespace pe { namespace scgi {

class Dispatcher;
class Pool;

class Server : noncopyable
{
  using DispatcherPtr = std::shared_ptr<Dispatcher>;
  using Listener = sockets::tcp::Server;

public:
  Server(const DispatcherPtr& dispatcher,
         const string& addr,
         uint16_t port,
         size_t maxSize=200);
  ~Server();

private:
  void accept();

private:
  DispatcherPtr dispatcher_;
  std::shared_ptr<Pool> pool_;
  std::shared_ptr<Listener> listener_;
};

} } //namespace pe { namespace scgi {

#endif //#ifndef _PE_SCGI_SERVER__20190228_
