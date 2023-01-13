#include "engine.h"

#include "dispatcher.h"
#include "server.h"
#include "trace.h"

namespace pe { namespace scgi {

PE_DECLARE_LOG_CHANNEL(THISLOG, "scgi.engine: ");

Engine::Engine(boost::asio::io_service& ios,
               const string& addr,
               uint16_t port,
               size_t maxSize)
  : dispatcher_(std::make_shared<Dispatcher>(ios))
  , server_(new Server(dispatcher(), addr, port, maxSize))
{
  PE_DEBUG(THISLOG << "created...");
}

Engine::~Engine()
{
  PE_DEBUG(THISLOG << "destroying...");
}

} } //namespace pe { namespace scgi {
