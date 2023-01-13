#ifndef _PE_PCAP_PARSERS_PACKET__H_20191003_
#define _PE_PCAP_PARSERS_PACKET__H_20191003_

#include "types.h"

namespace pe { namespace pcap { namespace parsers {

struct PacketInfo
{
  struct IPVersion
  {
    enum
    {
      kUNKNOWN = 0,
      kIPv4 = 4,
      kIPv6 = 6
    };
  };

  struct IPProto
  {
    enum
    {
      kIP = 0,
      kTCP = 6,
      kUDP = 17
    };
  };

  int ip = IPVersion::kUNKNOWN;
  int proto = IPProto::kIP;
  timeval ts = { 0, 0 };
  string srcAddr;
  uint16_t srcPort = 0;
  string dstAddr;
  uint16_t dstPort = 0;
};

class Packet
{
public:
  using data_t = string;
  using data_ptr = std::shared_ptr<data_t>;
  using pointer_t = std::shared_ptr<Packet>;

public:
  Packet(const data_ptr& data,
         const PacketInfo& pi=PacketInfo())
    : data_(data)
    , info_(pi)
  {
  }

  Packet(const string& str,
         const PacketInfo& pi=PacketInfo())
    : data_(new string(str))
    , info_(pi)
  {
  }

  Packet(string&& str,
         const PacketInfo& pi=PacketInfo())
    : data_(new string(str))
    , info_(pi)
  {
  }

  Packet(const char* buf, size_t len,
         const PacketInfo& pi=PacketInfo())
    : data_(new string(buf, len))
    , info_(pi)
  {
  }

public:
  const data_ptr& data() const { return data_; }
  const PacketInfo& info() const { return info_; }
  void info(const PacketInfo& value) { info_ = value; }

private:
  data_ptr data_;
  PacketInfo info_;
};

using PacketPtr = Packet::pointer_t;

} } } //namespace pe { namespace pcap { namespace parsers {

#endif //_PE_PCAP_PARSERS_PACKET__H_20191003_
