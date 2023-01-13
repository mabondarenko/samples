#include "tcpreassembler.h"

#include <pcapplusplus/TcpReassembly.h>

namespace pe { namespace pcap { namespace parsers {

TCPReassembler::TCPReassembler(int datalink)
  : _Base(datalink)
  , reasm_(create())
{
}

TCPReassembler::~TCPReassembler()
{
  //for forwards & std::unique_ptr<Reassembler> only
}

TCPReassembler::Reassembler* TCPReassembler::create()
{
  auto fOnMsgReady = [](int side, const pcpp::TcpStreamData& tcpData, void* userCookie)
  {
    PE_UNUSED(side);

    TCPReassembler* self = static_cast<TCPReassembler*>(userCookie);

    self->data_ = reinterpret_cast<const char*>(tcpData.getData());
    self->len_ = tcpData.getDataLength();

    PacketInfo* pi = &self->info_;
    pi->proto = PacketInfo::IPProto::kTCP;

    const auto& cd = tcpData.getConnectionData();

    pi->srcPort = cd.srcPort;
    pi->dstPort = cd.dstPort;

    if (cd.srcIP)
    {
      bool ipv6 = cd.srcIP->getType()==pcpp::IPAddress::IPv6AddressType;
      pi->ip = ipv6 ? PacketInfo::IPVersion::kIPv6 : PacketInfo::IPVersion::kIPv4;
      pi->srcAddr = cd.srcIP->toString();
    }

    if (cd.dstIP)
      pi->dstAddr = cd.dstIP->toString();
  };

  return new pcpp::TcpReassembly(fOnMsgReady, this);
}

TCPReassembler::PacketsList TCPReassembler::packets(const char* data,
                                                   size_t len,
                                                   const timeval& ts)
{
  pcpp::RawPacket raw(reinterpret_cast<const uint8_t*>(data),
                      static_cast<int>(len),
                      ts,
                      false,
                      static_cast<pcpp::LinkLayerType>(datalink()));
  reasm_->reassemblePacket(&raw);

  PacketsList result;

  if (data_)
  {
    info_.ts = ts;

    result.push_back(std::make_shared<Packet>(data_, len_, info_));

    //reset all internal data
    data_ = nullptr;
    len_ = 0;
    info_ = PacketInfo();
  }

  return result;
}

} } } //namespace pe { namespace pcap { namespace parsers {
