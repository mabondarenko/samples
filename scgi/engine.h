#ifndef _PE_SCGI_ENGINE__20190226_
#define _PE_SCGI_ENGINE__20190226_

#include "types.h"

#include <boost/asio/io_service.hpp>

namespace pe { namespace scgi {

class Dispatcher;
class Server;

class Engine : noncopyable
{
  using DispatcherPtr = std::shared_ptr<Dispatcher>;

public:
  Engine(boost::asio::io_service& ios,
         const string& addr,
         uint16_t port=9080,
         size_t maxSize=2000);
  ~Engine();

public:
  const DispatcherPtr& dispatcher() const { return dispatcher_; }

private:
  DispatcherPtr dispatcher_;
  std::unique_ptr<Server> server_;
};

} } //namespace pe { namespace scgi {

#endif //#ifndef _PE_SCGI_ENGINE__20190226_
