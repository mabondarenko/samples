#include "raw.h"

namespace pe { namespace pcap { namespace parsers {

Raw::PacketsList Raw::packets(const char* data, size_t len, const timeval& ts)
{
  PacketInfo info;
  info.ts = ts;

  PacketsList result;

  auto p = payload(data, len, info);
  if (p < data + len)
    result.push_back(std::make_shared<Packet>(p, data + len - p, info));

  return result;
}

} } } //namespace pe { namespace pcap { namespace parsers {