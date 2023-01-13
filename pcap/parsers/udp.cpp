#include "udp.h"

#include <netinet/udp.h>
#include <netinet/in.h>

namespace pe { namespace pcap { namespace parsers {

const char* UDP::payload(const char* data, size_t len, PacketInfo& info)
{
  auto p = _Base::payload(data, len, info);

  if (p + sizeof(udphdr) < data + len)
  {
    const udphdr* udp = reinterpret_cast<const udphdr*>(p);
    if (p + htons(udp->len) <= data + len)
    {
      info.srcPort = htons(udp->source);
      info.dstPort = htons(udp->dest);
      return p + sizeof(udphdr);
    }
  }

  return data + len;
}

} } } //namespace pe { namespace pcap { namespace parsers {
