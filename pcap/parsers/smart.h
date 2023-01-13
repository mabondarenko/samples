#ifndef _PE_PCAP_PARSERS_SMART__H_20191115_
#define _PE_PCAP_PARSERS_SMART__H_20191115_

#include "ip.h"

namespace pe { namespace pcap { namespace parsers {

class Smart : public IP
{
  using _Base = IP;

public:
  Smart(int datalink) : _Base(datalink) {}

public:
  virtual const char* payload(const char* data, size_t len, PacketInfo& info) override;
};

} } } //namespace pe { namespace pcap { namespace parsers {

#endif //#ifndef _PE_PCAP_PARSERS_SMART__H_20191115_