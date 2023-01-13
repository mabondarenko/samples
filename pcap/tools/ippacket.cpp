#include "ippacket.h"

#include <atomic>
#include <cstring>

#include <arpa/inet.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/udp.h>
#include <pcap/dlt.h>

namespace pe { namespace pcap { namespace tools {

IPPacket::IPPacket(int _datalink,
                   int _af,
                   int _proto,
                   const string& _srcAddr,
                   uint16_t _srcPort,
                   const string& _dstAddr,
                   uint16_t _dstPort,
                   const char* _data,
                   size_t _dataSize,
                   const uint16_t* _srcMAC,
                   const uint16_t* _dstMAC)
  : datalink_(_datalink), af_(_af), protocol_(_proto)
  , srcAddr_(_srcAddr), srcPort_(_srcPort)
  , dstAddr_(_dstAddr), dstPort_(_dstPort)
  , data_(_data), dataSize_(_dataSize)
  , srcMAC_(_srcMAC), dstMAC_(_dstMAC)
{
  protocol_ = IPPROTO_UDP; //TODO: !!!!!!!!!

  memset(pseudo_, 0, sizeof(pseudo_));

  currentOffset_ += writeDataLink(next());
  currentOffset_ += writeIPHeader(next());
  currentOffset_ += writeProtoHeader(next());
  finalize();
}

std::ostream& IPPacket::serialize(std::ostream& out) const
{
  if (!out.write(buf_, currentOffset_).good())
    return out;

  return out.write(data(), dataSize());
}

void IPPacket::finalize()
{
  if (af_ == AF_INET)
  {
    auto ip = reinterpret_cast<iphdr*>(ipHdr());
    ip->tot_len = htons(static_cast<uint16_t>(size() - ipOffset_));
    ip->check = calcCheckSumm(ip, sizeof(*ip));
  }
  else if (af_ == AF_INET6)
  {
    auto ip = reinterpret_cast<ip6_hdr*>(ipHdr());
    ip->ip6_ctlun.ip6_un1.ip6_un1_plen = htons(static_cast<uint16_t>(size() - ipOffset_));

  }
}

size_t IPPacket::writeProtoHeader(void* dst)
{
  switch(protocol_)
  {
  case IPPROTO_UDP:
    return writeUDPHeader(dst);
  case IPPROTO_TCP:
    return writeTCPHeader(dst);
  default:
    throw exceptions::runtime_error("IPPacket: unsupported IP protocol=" + std::to_string(protocol_));
  }
  return 0;
}

size_t IPPacket::writeUDPHeader(void* dst)
{
  auto udp = reinterpret_cast<udphdr*>(dst);

  udp->source = htons(srcPort_);
  udp->dest = htons(dstPort_);
  udp->len = htons(static_cast<uint16_t>(sizeof(*udp) + dataSize()));
  udp->check = 0;

  auto x16 = 0;
  if (af_ == AF_INET)
  {
    pseudo_->ip4.length = udp->len;
    x16 = calcCheckSumm(&pseudo_->ip4, sizeof(pseudo_->ip4));
  }
  else if (af_ == AF_INET6)
  {
    x16 = calcCheckSumm(&pseudo_->ip6, sizeof(pseudo_->ip6));
  }

  auto u = htons(x16);

  x16 = calcCheckSumm(udp, sizeof(*udp));
  u += htons(x16);

  x16 = calcCheckSumm(data(), dataSize());
  u += htons(x16);

  x16  = (u & 0xffff) + (u>>16);
  udp->check = htons(x16);

  if (udp->check==0)
    udp->check = htons(1);

  return sizeof(udphdr);
}

size_t IPPacket::writeTCPHeader(void* dst)
{
  PE_UNUSED(dst);

  throw exceptions::runtime_error("IPPacket: IPPROTO_TCP not implemented yet");

  return 0;
}

size_t IPPacket::writeIPHeader(void* dst)
{
  ipOffset_ = currentOffset_;

  if (af_ == AF_INET)
  {
    auto ip = reinterpret_cast<iphdr*>(dst);

    if (!getIPAddr(srcAddr_, &ip->saddr))
      throw exceptions::runtime_error("IPPacket: bad IPv4 src address: " + srcAddr_);
    if (!getIPAddr(dstAddr_, &ip->daddr))
      throw exceptions::runtime_error("IPPacket: bad IPv4 dst address: " + dstAddr_);

    ip->version = 4;
    ip->ihl = sizeof(iphdr) >> 2;
    ip->protocol = static_cast<uint8_t>(protocol_);

    ip->tos = 0;
    ip->tot_len = 0;
    ip->id = htons(static_cast<uint16_t>(flowSeq()));
    ip->frag_off = 0;
    ip->ttl = 0xff;
    ip->check = 0;

    //fill pseudo for UDP & TCP checksum calculation:
    pseudo_->ip4.srcAddr = ip->saddr;
    pseudo_->ip4.dstAddr = ip->daddr;
    pseudo_->ip4.protocol = ip->protocol;

    return sizeof(iphdr);
  }
  else if (af_ == AF_INET6)
  {
    auto ip = reinterpret_cast<ip6_hdr*>(dst);

    if (!getIPAddr(srcAddr_, &ip->ip6_src))
      throw exceptions::runtime_error("IPPacket: bad IPv6 src address: " + srcAddr_);
    if (!getIPAddr(dstAddr_, &ip->ip6_dst))
      throw exceptions::runtime_error("IPPacket: bad IPv6 dst address: " + dstAddr_);

    ip->ip6_ctlun.ip6_un1.ip6_un1_plen = 0;
    ip->ip6_ctlun.ip6_un1.ip6_un1_nxt = static_cast<uint8_t>(protocol_);
    ip->ip6_ctlun.ip6_un1.ip6_un1_hlim = 32;

    union ip6_un1_flow_t
    {
      struct {
        uint32_t tc1:4;
        uint32_t version:4;
        uint32_t tc2:4;
        uint32_t flow:20;
      } u;
      uint32_t flow;
    } tmp;

    tmp.flow = 0;
    tmp.u.version = 6;
    tmp.u.flow = htonl(flowSeq()&0xFFFFFF);

    ip->ip6_ctlun.ip6_un1.ip6_un1_flow = tmp.flow;

    //fill pseudo for UDP & TCP checksum calculation
    memcpy(&pseudo_->ip6.srcAddr, &ip->ip6_src, std::min(sizeof(pseudo_->ip6.srcAddr), sizeof(ip->ip6_src)));
    memcpy(&pseudo_->ip6.dstAddr, &ip->ip6_dst, std::min(sizeof(pseudo_->ip6.dstAddr), sizeof(ip->ip6_dst)));
    pseudo_->ip6.protocol = ip->ip6_ctlun.ip6_un1.ip6_un1_nxt;

    return sizeof(ip6_hdr);
  }
  else
  {
    throw exceptions::runtime_error("IPPacket: unsupported address family=" + std::to_string(af_));
  }

  return 0;
}

size_t IPPacket::writeDataLink(void* dst)
{
  //https://www.tcpdump.org/linktypes.html

  switch(datalink_)
  {
  case DLT_RAW:
    return 0;

  case DLT_NULL:
  case DLT_LOOP:
    {
      auto hdr = reinterpret_cast<uint32_t*>(dst);
      if (af_ == AF_INET)
        *hdr = 2;
      else if (af_ == AF_INET6)
        *hdr = 24;
      else
        *hdr = AF_UNSPEC;

      if (datalink_==DLT_LOOP)
        *hdr = htonl(*hdr);
    }
    return sizeof(uint32_t);

  case DLT_EN10MB:
    {
      auto hdr = reinterpret_cast<ether_header*>(dst);

      static const uint16_t __srcMAC[ETH_ALEN] = { 0x00, 0x16, 0x3e, 0xd6, 0xfa, 0xfb };
      static const uint16_t __dstMAC[ETH_ALEN] = { 0x00, 0x1f, 0x12, 0xb9, 0xb7, 0xf0 };

      //static const uint16_t __srcMAC[ETH_ALEN] = { 0xbf, 0xfa, 0xd6, 0x3e, 0x16, 0x00 };
      //static const uint16_t __dstMAC[ETH_ALEN] = { 0xf0, 0xb7, 0xb9, 0x12, 0x1f, 0x00 };

      memcpy(hdr->ether_shost, srcMAC_ ? srcMAC_ : __srcMAC, ETH_ALEN);
      memcpy(hdr->ether_dhost, dstMAC_ ? dstMAC_ : __dstMAC, ETH_ALEN);

      if (af_ == AF_INET)
        hdr->ether_type = htons(ETHERTYPE_IP);
      else if (af_ == AF_INET6)
        hdr->ether_type = htons(ETHERTYPE_IPV6);
      else
        hdr->ether_type = htons(ETHERTYPE_LOOPBACK);
    }
    return sizeof(ether_header);

  default:
    throw exceptions::runtime_error("IPPacket: unsupported datalink=" + std::to_string(datalink_));
  }

  return 0;
}

size_t IPPacket::dataLinkSize(int datalink)
{
  switch(datalink)
  {
  case DLT_RAW:
    return 0;
  case DLT_NULL:
  case DLT_LOOP:
    return sizeof(uint32_t);
  case DLT_EN10MB:
    return sizeof(ether_header);
  default:
    throw exceptions::runtime_error("IPPacket: unsupported datalink=" + std::to_string(datalink));
  }
  return 0;
}

uint16_t IPPacket::calcCheckSumm(const void* data, size_t size)
{
  uint32_t summ = 0;

  size_t sz = size;
  auto p = reinterpret_cast<const uint16_t*>(data);

  while(sz>=sizeof(uint16_t))
  {
    summ += htons(*p++);
    sz -= sizeof(uint16_t);
  }

  if (sz==1)
  {
    auto b = *reinterpret_cast<const uint8_t*>(p);
    summ += htons(b);
  }

  while(summ>>16)
    summ = (summ & 0xffff) + (summ >> 16);

  summ = ~summ;

  return htons(static_cast<uint16_t>(summ));
}

bool IPPacket::getIPAddr(const string& src, void* dst) const
{
  return inet_pton(af_, src.data(), dst)>0;
}

uint32_t IPPacket::flowSeq()
{
  static std::atomic<uint32_t> seqno(0xabcdd);
  return ++seqno;
}

} } } //namespace pe { namespace pcap { namespace tools {