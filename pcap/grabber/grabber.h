#ifndef _PE_PCAP_GRABBER_GRABBER__H_20191112_
#define _PE_PCAP_GRABBER_GRABBER__H_20191112_

#include "types.h"

#include <boost/asio/io_service.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>

#include <nlohmann/json.hpp>

#include "livecapture/grabber.h"
#include "tools/signals.h"
#include "config.h"

namespace pe { namespace pcap { namespace grabber {

class Sniffer;

class Grabber : public livecapture::Grabber
{
  using _Base = livecapture::Grabber;

public:
  using json = nlohmann::json;

public:
  Grabber(boost::asio::io_service& ios,
          const string& subscriberId,
          const string& ip,
          const json& config);
  Grabber(boost::asio::io_service& ios,
          const string& subscriberId,
          const Config& config);
  virtual ~Grabber() override;

protected:
  virtual SnifferPtr createSniffer(const string& capture=string()) override;
  virtual bool isCaptureExists(const string& capture) const override;
  virtual const string& listenInterface() const override;
  virtual uint16_t listenMinPort() const override;
  virtual uint16_t listenMaxPort() const override;
  virtual size_t connectionsLimit() const override;
  virtual uint64_t listenTimeout() const override;

private:
  const Config config_;
};

} } } //namespace pe { namespace pcap { namespace grabber {

#endif //_PE_PCAP_GRABBER_GRABBER__H_20191112_