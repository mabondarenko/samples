#include "ip.h"

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>

namespace pe { namespace pcap { namespace parsers {

const char* IP::payload(const char* data, size_t len, PacketInfo& info)
{
  auto p = _Base::payload(data, len, info);

  if (p + sizeof(iphdr) < data + len)
  {
    const iphdr* ip = reinterpret_cast<const iphdr*>(p);
    if (ip->version==4)
    {
      size_t offset = ip->ihl << 2; // x4
      if (p + offset < data + len)
      {
        info.ip = ip->version;
        info.proto = ip->protocol;
        info.srcAddr = ipv4(&ip->saddr);
        info.dstAddr = ipv4(&ip->daddr);

        return p + offset;
      }
    }
    else if (ip->version==6)
    {
      const ip6_hdr* ip6 = reinterpret_cast<const ip6_hdr*>(p);
      size_t offset = sizeof(ip6_hdr); //TODO: IPv6

      if (p + offset < data + len)
      {
        info.ip = ip->version;
        info.proto = ip6->ip6_ctlun.ip6_un1.ip6_un1_nxt;
        info.srcAddr = ipv6(&ip6->ip6_src);
        info.dstAddr = ipv6(&ip6->ip6_dst);

        return p + offset;
      }
    }
  }

  return data + len;
}

string IP::ipv4(const void* addr)
{
  char dst[INET_ADDRSTRLEN];
  dst[0] = 0;
  const auto* p = inet_ntop(AF_INET, addr, dst, INET_ADDRSTRLEN);

  return p ? string(p) : string();
}

string IP::ipv6(const void* addr)
{
  char dst[INET6_ADDRSTRLEN];
  dst[0] = 0;
  const auto* p = inet_ntop(AF_INET6, addr, dst, INET6_ADDRSTRLEN);
  return p ? string(p) : string();
}

} } } //namespace pe { namespace pcap { namespace parsers {
