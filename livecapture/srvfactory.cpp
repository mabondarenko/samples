#include "srvfactory.h"

#include "servertcp.h"
#include "serverws.h"

namespace pe { namespace livecapture {

ServerFactory::ServerPtr ServerFactory::create(const string& transport,
                                               const Server::GrabberPtr& grabber,
                                               const Server::Params& params,
                                               Server::Terminate* terminate)
{
  if (transport=="tcp")
    return std::make_shared<ServerTCP>(grabber, params, terminate);

  if (transport=="websocket")
    return std::make_shared<ServerWS>(grabber, params, terminate);

  return ServerPtr();
}

} } //namespace pe { namespace livecapture {