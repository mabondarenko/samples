#include "tcp.h"

#include <netinet/tcp.h>
#include <netinet/in.h>

namespace pe { namespace pcap { namespace parsers {

const char* TCP::payload(const char* data, size_t len, PacketInfo& info)
{
  auto p = _Base::payload(data, len, info);

  if (p + sizeof(tcphdr) < data + len)
  {
    const tcphdr* tcp = reinterpret_cast<const tcphdr*>(p);
    size_t offset = tcp->th_off << 2; //x4
    if (p + offset < data + len)
    {
      info.srcPort = htons(tcp->th_sport);
      info.dstPort = htons(tcp->th_dport);
      return p + offset;
    }
  }

  return data + len;
}

} } } //namespace pe { namespace pcap { namespace parsers {
