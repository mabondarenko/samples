#include "servertcp.h"

#include "sockets/tcp/connection.h"
#include "sockets/tcp/server.h"
#include "connection.h"
#include "grabber.h"
#include "trace.h"

namespace pe { namespace livecapture {

PE_DECLARE_LOG_CHANNEL(SRV_LOG, "live.tcp[");
#define THISLOG SRV_LOG << port() << "]: "

ServerTCP::ServerTCP(const GrabberPtr& _grabber,
                     const Params& p,
                     Terminate* terminate)
  : _Base(_grabber, p, terminate)
  , listener_(std::make_shared<Listener>(_grabber->ios(), p.minport, p.maxport, p.interface))
{
  PE_DEBUG(THISLOG << "constructed...");
}

ServerTCP::~ServerTCP()
{
  PE_DEBUG(THISLOG << "destroying...");
}

void ServerTCP::cancel()
{
  listener_->cancel();
}

uint16_t ServerTCP::port() const
{
  return listener_->port();
}

void ServerTCP::accept()
{
  auto self = shared_from_this();
  auto handler = [this, self](const Listener::SocketPtr& s) -> bool
  {
    bool result = false;

    PE_DEBUG(THISLOG << "incoming connection from '"
                     << s->remote_endpoint().address().to_string()
                     << ":" << s->remote_endpoint().port() << "'...");

    if (count_!=0)
    {
      PE_ERROR(THISLOG << "no more connections are allowed on this port!");
    }
    else if (auto gr = grabber())
    {
      try
      {
        auto connection = std::make_shared<Connection>(std::make_shared<sockets::tcp::Connection>(listener_->ios(), s),
                                                       gr->sniffer(params().name),
                                                       params().filter,
                                                       &onterminate());
        connectOnFinish(connection->onterminate());
        connection->start(params().period);

        stopAcceptTimer();
        ++count_;

        result = true;
      }
      catch(const std::exception& e)
      {
        PE_ERROR(THISLOG << "failed to create client connection: " << e.what());
      }
    }
    else
    {
      PE_ERROR(THISLOG << "capture is no longer available");
    }

    //we continue to accept connections to hold the TCP port in the system,
    //even if connections are no longer allowed
    accept();

    return result;
  };

  listener_->accept(handler);
}

} } //namespace pe { namespace livecapture {