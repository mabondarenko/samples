#include "datalink.h"

#include <netinet/ether.h>
#include <pcap/dlt.h>
#include <pcap/sll.h>

namespace pe { namespace pcap { namespace parsers {

size_t Datalink::headerSize(size_t len) const
{
  //https://www.tcpdump.org/linktypes.html

  switch(datalink())
  {
  case DLT_RAW:
    return 0;

  case DLT_NULL:
  case DLT_LOOP:
    return 4;

  case DLT_EN10MB:
    return sizeof(ether_header);

  case DLT_LINUX_SLL:
    return sizeof(sll_header);

  default:
    break;
  }

  return len;
}

const char* Datalink::payload(const char* data, size_t len, PacketInfo& info)
{
  auto p = _Base::payload(data, len, info);

  auto offset = headerSize(len);
  if (p + offset < data + len)
    return p + offset;

  return data + len;
}

} } } //namespace pe { namespace pcap { namespace parsers {
