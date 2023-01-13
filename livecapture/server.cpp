#include "server.h"

#include "grabber.h"
#include "trace.h"

namespace pe { namespace livecapture {

PE_DECLARE_LOG_CHANNEL(SRV_LOG, "live.server[");
#define THISLOG SRV_LOG << port() << "]: "

Server::Server(const GrabberPtr& _grabber,
               const Params& p,
               Terminate* terminate)
  : grabber_(_grabber)
  , tmrAccept_(new timer_t(_grabber->ios()))
  , params_(p.capture)
  , termination_(connect(terminate))
{
}

Server::~Server()
{
  finish_.disconnect();
  termination_.disconnect();
  terminate_();
}

signals::scoped Server::connect(signals::simple& sig)
{
  return sig.connect_extended([this](const signals::connection& c){
    PE_DEBUG(THISLOG << "termination signal received!");
    c.disconnect();
    stopAcceptTimer();
    cancel();
  });
}

signals::scoped Server::connect(signals::simple* sig)
{
  return sig ? connect(*sig) : signals::scoped();
}

void Server::connectOnFinish(signals::simple& sig)
{
  finish_  = connect(sig);
}

void Server::start(uint64_t seconds)
{
  PE_DEBUG(THISLOG << "starting listen port for " << seconds << " seconds...");

  wait(seconds);
  accept();
}

void Server::wait(uint64_t seconds)
{
  guard_t g(mtx_);

  if (!tmrAccept_)
  {
    PE_ERROR(THISLOG << "waiting impossible...");
    return;
  }

  tmrAccept_->expires_from_now(boost::posix_time::seconds(seconds));

  auto self = shared_from_this();
  tmrAccept_->async_wait([self, this](const boost::system::error_code& error) {
    if (!error)
    {
      PE_DEBUG(THISLOG << "accept timer expired");

      stopAcceptTimer();
      cancel();
    }
  });
}

void Server::stopAcceptTimer()
{
  guard_t g(mtx_);
  tmrAccept_.reset();
}

} } //namespace pe { namespace livecapture {