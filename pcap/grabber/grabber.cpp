#include "grabber.h"

#include "sniffer.h"
#include "trace.h"

namespace pe { namespace pcap { namespace grabber {

PE_DECLARE_LOG_CHANNEL(THISLOG, "pcap.grabber: ");

Grabber::Grabber(boost::asio::io_service& ios,
                 const string& subscriberId,
                 const string& ip,
                 const json& config)
  : Grabber(ios, subscriberId, Config(ip, config))
{
}

Grabber::Grabber(boost::asio::io_service& ios,
                 const string& subscriberId,
                 const Config& config)
  : _Base(ios, subscriberId)
  , config_(config)
{
  PE_DEBUG(THISLOG << "created...");
}

Grabber::~Grabber()
{
  PE_DEBUG(THISLOG << "destroying...");
}

Grabber::SnifferPtr Grabber::createSniffer(const string& capture)
{
  auto* pcapture = config_.capture(capture);
  if (!pcapture || !pcapture->enabled)
    return SnifferPtr();

  Sniffer::Params params
  {
    capture,
    pcapture->interface,
    pcapture->filter,
    pcapture->parser
  };

  return std::make_shared<Sniffer>(ios(), params);
}

bool Grabber::isCaptureExists(const string& capture) const
{
  auto* pcapture = config_.capture(capture);
  return pcapture && pcapture->enabled;
}

const string& Grabber::listenInterface() const
{
  return config_.listener.interface;
}

uint16_t Grabber::listenMinPort() const
{
  return config_.listener.minport;
}

uint16_t Grabber::listenMaxPort() const
{
  return config_.listener.maxport;
}

size_t Grabber::connectionsLimit() const
{
  return config_.listener.limit;
}

uint64_t Grabber::listenTimeout() const
{
  return config_.listener.timeout;
}

} } } //namespace pe { namespace pcap { namespace grabber {
