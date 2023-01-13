#ifndef _PE_PCAP_PARSERS_TCP__H_20191109_
#define _PE_PCAP_PARSERS_TCP__H_20191109_

#include "ip.h"

namespace pe { namespace pcap { namespace parsers {

class TCP : public IP
{
  using _Base = IP;

public:
  TCP(int datalink) : _Base(datalink) {}

public:
  virtual const char* payload(const char* data, size_t len, PacketInfo& info) override;
};

} } } //namespace pe { namespace pcap { namespace parsers {

#endif //#ifndef _PE_PCAP_PARSERS_TCP__H_20191109_