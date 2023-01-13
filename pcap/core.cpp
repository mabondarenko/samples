#include "core.h"

#include "grabber/grabber.h"
#include "tools/jsonhelper.h"
#include "trace.h"

namespace pe { namespace pcap {

PE_DECLARE_LOG_CHANNEL(THISLOG, "pcap.core: ");

Core::Core(const string& _config)
  : _Base(_config)
  , loop_(jhlp::getValue<size_t>(config().at("pcap"), "threads", 4), "pcap.core")
  , grabber_(std::make_shared<Grabber>(ios(),
                                       subscriberId(),
                                       hostIP(),
                                       jhlp::getValue<json>(config().at("pcap"), "live")))
{
  PE_DEBUG(THISLOG << "constructed...");
}

Core::~Core()
{
  PE_DEBUG(THISLOG << "destroing...");
}

} } //namespace pe { namespace pcap {