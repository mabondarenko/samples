#ifndef _PE_LIVECAPTURE_SRVFACTORY__H_20200712_
#define _PE_LIVECAPTURE_SRVFACTORY__H_20200712_

#include "server.h"

namespace pe { namespace livecapture {

struct ServerFactory
{
  using ServerPtr = std::shared_ptr<Server>;

  static ServerPtr create(const string& transport,
                          const Server::GrabberPtr& grabber,
                          const Server::Params& params,
                          Server::Terminate* terminate=nullptr);
};

} } //namespace pe { namespace livecapture {

#endif //_PE_LIVECAPTURE_SRVFACTORY__H_20200712_