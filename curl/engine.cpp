#include "engine.h"

#include "service.h"

//#define _TEST_ENGINE_SHUTDOWN

#ifdef _TEST_ENGINE_SHUTDOWN
#include "remotedir.h"
#endif

#include "trace.h"

namespace pe { namespace curl {

PE_DECLARE_LOG_CHANNEL(ENGINE_LOG, "curl.engine");
#define THISLOG ENGINE_LOG << "[" << name() << "]: "

Engine::Engine(const string& _name)
  : service_(std::make_shared<Service>(ios(), _name))
  , work_(new boost::asio::io_service::work(ios()))
{
  PE_DEBUG(THISLOG << "creating...");

  worker_ = boost::thread(&Engine::work, this);
}

Engine::~Engine(void)
{
  PE_DEBUG(THISLOG << "destroying...");

#ifdef _TEST_ENGINE_SHUTDOWN
  test();
#endif

  service_->stop();

  work_.reset();
  worker_.join();

  PE_DEBUG(THISLOG << "destroyed...");
}

const string& Engine::name(void) const
{
  return service()->name();
}

void Engine::work(void)
{
  PE_TRACE(THISLOG << "I/O loop starting...");

  ios_.run();

  PE_TRACE(THISLOG << "I/O loop exited...");
}

void Engine::test(void)
{
#ifdef _TEST_ENGINE_SHUTDOWN
  PE_INFO(THISLOG << "performing shutdown test with RemoteDirectory...");
  auto rd = std::make_shared<RemoteDirectory>(service(), "ftp://speedtest.tele2.net/");
  rd->exec([rd, this](const Transfer::pointer_t&, int error, const char* text) {
    if (!error)
    {
      PE_INFO(THISLOG << "shutdown test RemoteDirectory:\n" << rd->data());
    }
    else
    {
      PE_ERROR(THISLOG << "shutdown test RemoteDirectory failed with error=" << error << ", text=" << text);
    }
  });
#endif
}

} } //namespace pe { namespace curl {
