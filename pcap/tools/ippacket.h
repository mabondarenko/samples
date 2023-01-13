#ifndef _PE_PCAP_TOOLS_IPPACKET__H_20200725_
#define _PE_PCAP_TOOLS_IPPACKET__H_20200725_

#include "types.h"

#include <ostream>

namespace pe { namespace pcap { namespace tools {

class IPPacket
{
public:
  IPPacket(int datalink,
           int af,
           int proto,
           const string& srcAddr,
           uint16_t srcPort,
           const string& dstAddr,
           uint16_t dstPort,
           const char* data,
           size_t dataSize,
           const uint16_t* srcMAC=nullptr,
           const uint16_t* dstMAC=nullptr);

public:
  const char* data() const { return data_; }
  size_t dataSize() const { return dataSize_; }
  size_t size() const { return currentOffset_ + dataSize(); }

public:
  std::ostream& serialize(std::ostream& out) const;

public:
  static size_t dataLinkSize(int datalink);
  static uint16_t calcCheckSumm(const void* data, size_t size);
  static uint32_t flowSeq();

protected:
  bool getIPAddr(const string& src, void* dst) const;
  size_t writeDataLink(void* dst);
  size_t writeIPHeader(void* dst);
  size_t writeProtoHeader(void* dst);
  size_t writeUDPHeader(void* dst);
  size_t writeTCPHeader(void* dst);
  void finalize();

protected:
  char* next() { return buf_ + currentOffset_; }
  void* ipHdr() { return buf_ + ipOffset_; }

  union PseudoIPHeader
  {
    struct IPv4
    {
      uint32_t srcAddr;
      uint32_t dstAddr;
      uint8_t  zero;
      uint8_t  protocol;
      uint16_t length;

    } ip4;

    struct IPv6
    {
      struct in6_addr
      {
        union
        {
          uint8_t __u6_addr8[16];
          uint16_t __u6_addr16[8];
          uint32_t __u6_addr32[4];
        } __in6_u;
      };

      in6_addr srcAddr;
      in6_addr dstAddr;
      uint32_t protocol;
      uint32_t zero;

    } ip6;
  };

private:
  int datalink_;
  int af_;
  int protocol_;
  const string& srcAddr_;
  uint16_t srcPort_;
  const string& dstAddr_;
  uint16_t dstPort_;
  const char* data_;
  size_t dataSize_;
  const uint16_t* srcMAC_ = nullptr;
  const uint16_t* dstMAC_ = nullptr;
  size_t currentOffset_ = 0;
  size_t ipOffset_ = 0;
  PseudoIPHeader pseudo_[1];
  char buf_[1024];
};

inline std::ostream& operator<<(std::ostream& os, const IPPacket& p) { return p.serialize(os); }

} } } //namespace pe { namespace pcap { namespace tools {

#endif //#ifndef _PE_PCAP_TOOLS_IPPACKET__H_20200725_