#ifndef _PE_PCAP_CORE__H_20191002_
#define _PE_PCAP_CORE__H_20191002_

#include "context/context.h"

#include "io/eventloop.h"

namespace pe { namespace pcap {

namespace grabber { class Grabber; }

class Core : public Context
{
  using _Base = Context;
  using Grabber = grabber::Grabber;
  using GrabberPtr = std::shared_ptr<Grabber>;

public:
  Core(const string& config);
  virtual ~Core() override;

public:
  boost::asio::io_service& ios() { return loop_.ios(); }
  const GrabberPtr& grabber() const { return grabber_; }

private:
  io::EventLoop loop_;
  GrabberPtr grabber_;
};

} } //namespace pe { namespace pcap {

#endif //_PE_PCAP_CORE__H_20191002_