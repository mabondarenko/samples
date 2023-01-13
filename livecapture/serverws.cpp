#include "serverws.h"

#include "sockets/web/connection.h"
#include "sockets/web/server.h"
#include "connection.h"
#include "grabber.h"
#include "trace.h"

namespace pe { namespace livecapture {

PE_DECLARE_LOG_CHANNEL(SRV_LOG, "live.websocket[");
#define THISLOG SRV_LOG << port() << "]: "

ServerWS::ServerWS(const GrabberPtr& _grabber,
                   const Params& p,
                   Terminate* terminate)
  : _Base(_grabber, p, terminate)
  , listener_(std::make_shared<Listener>(_grabber->ios(), p.minport, p.maxport, p.interface))
{
  PE_DEBUG(THISLOG << "constructed...");
}

ServerWS::~ServerWS()
{
  PE_DEBUG(THISLOG << "destroying...");
}

void ServerWS::cancel()
{
  listener_->cancel();
}

uint16_t ServerWS::port() const
{
  return listener_->port();
}

void ServerWS::accept()
{
  auto self = shared_from_this();
  auto handler = [this, self](const Listener::SocketPtr& s) -> bool
  {
    bool result = false;

    PE_DEBUG(THISLOG << "incoming connection from '"
                     << s->get_remote_endpoint() << "'...");

    if (count_!=0)
    {
      PE_ERROR(THISLOG << "no more connections are allowed on this port!");

    }
    else if (auto gr = grabber())
    {
      try
      {
        auto connection = std::make_shared<Connection>(std::make_shared<sockets::web::Connection>(listener_->ios(), s),
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

    //we continue to accept connections to hold the WebSocket port in the system,
    //even if connections are no longer allowed
    accept();

    return result;
  };

  listener_->accept(handler);
}

} } //namespace pe { namespace livecapture {