#include "smart.h"

#include <netinet/in.h>

#include "tcp.h"
#include "udp.h"

namespace pe { namespace pcap { namespace parsers {

const char* Smart::payload(const char* data, size_t len, PacketInfo& info)
{
  auto p = _Base::payload(data, len, info);

  switch(info.proto)
  {
  case IPPROTO_TCP:
    return TCP(datalink()).payload(data, len, info);

  case IPPROTO_UDP:
    return UDP(datalink()).payload(data, len, info);

  default:
    break;
  }

  return p;
}

} } } //namespace pe { namespace pcap { namespace parsers {
