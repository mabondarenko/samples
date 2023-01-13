#ifndef _PE_LIVECAPTURE_SERVERTCP__H_20200712_
#define _PE_LIVECAPTURE_SERVERTCP__H_20200712_

#include "server.h"

//forwards:
namespace pe { namespace sockets { namespace tcp { class Server; } } }

namespace pe { namespace livecapture {

class ServerTCP : public Server
{
  using _Base = Server;

public:
  ServerTCP(const GrabberPtr& grabber,
            const Params& params,
            Terminate* terminate=nullptr);
  virtual ~ServerTCP() override;

public:
  virtual uint16_t port() const override;

protected:
  virtual void accept() override;
  virtual void cancel() override;

private:
  using Listener = sockets::tcp::Server;
  using ListenerPtr = std::shared_ptr<Listener>;

private:
  ListenerPtr listener_;
  size_t count_ = 0;
};

} } //namespace pe { namespace livecapture {

#endif //_PE_LIVECAPTURE_SERVERTCP__H_20200712_