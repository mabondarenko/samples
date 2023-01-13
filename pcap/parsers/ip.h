#ifndef _PE_PCAP_PARSERS_IP__H_20191115_
#define _PE_PCAP_PARSERS_IP__H_20191115_

#include "datalink.h"

namespace pe { namespace pcap { namespace parsers {

class IP : public Datalink
{
  using _Base = Datalink;

public:
  IP(int datalink) : _Base(datalink) {}

public:
  virtual const char* payload(const char* data, size_t len, PacketInfo& info) override;

private:
  static string ipv4(const void* addr);
  static string ipv6(const void* addr);
};

} } } //namespace pe { namespace pcap { namespace parsers {

#endif //#ifndef _PE_PCAP_PARSERS_IP__H_20191115_