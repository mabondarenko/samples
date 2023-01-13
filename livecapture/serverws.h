#ifndef _PE_LIVECAPTURE_SERVERWS__H_20200712_
#define _PE_LIVECAPTURE_SERVERWS__H_20200712_

#include "server.h"

//forwards:
namespace pe { namespace sockets { namespace web { class Server; } } }

namespace pe { namespace livecapture {

class ServerWS : public Server
{
  using _Base = Server;

public:
  ServerWS(const GrabberPtr& grabber,
           const Params& params,
           Terminate* terminate=nullptr);
  virtual ~ServerWS() override;

public:
  virtual uint16_t port() const override;

protected:
  virtual void accept() override;
  virtual void cancel() override;

private:
  using Listener = sockets::web::Server;
  using ListenerPtr = std::shared_ptr<Listener>;

private:
  ListenerPtr listener_;
  size_t count_ = 0;
};

} } //namespace pe { namespace livecapture {

#endif //_PE_LIVECAPTURE_SERVERWS__H_20200712_