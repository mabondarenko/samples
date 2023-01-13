#ifndef _PE_PCAP_PARSER_MESSAGE__H_20200719_
#define _PE_PCAP_PARSER_MESSAGE__H_20200719_

#include "packet.h"
#include "livecapture/packet.h"

namespace pe { namespace pcap { namespace parsers {

class Message : public livecapture::Packet
{
public:
  Message(const PacketPtr& packet);

protected:
  Message(const PacketPtr& packet,
          const timeval& ts);

public:
  virtual int afamily() const override;
  virtual int ipproto() const override;
  virtual int protocol() const override { return 0; }
  virtual bool isIPv4() const override { return packet_->info().ip==PacketInfo::IPVersion::kIPv4; }
  virtual bool isIPv6() const override { return packet_->info().ip==PacketInfo::IPVersion::kIPv6; }
  virtual bool isUDP() const override { return packet_->info().proto==PacketInfo::IPProto::kUDP; }
  virtual bool isTCP() const override { return packet_->info().proto==PacketInfo::IPProto::kTCP; }
  virtual bool isSIP() const override { return false; }
  virtual bool isRTP() const override { return false; }
  virtual const data_ptr& compressed() const override { return data(); }
  virtual const data_ptr& data() const override { return packet_->data(); }
  virtual const string& srcAddr() const override { return packet_->info().srcAddr; }
  virtual uint16_t srcPort() const override { return packet_->info().srcPort; }
  virtual const string& dstAddr() const override { return packet_->info().dstAddr; }
  virtual uint16_t dstPort() const override { return packet_->info().dstPort; }
  virtual const timeval& timestamp() const override { return timestamp_; }
  virtual const string& correlationID() const override { static const string foo; return foo; }
  virtual pointer_t clone(bool touch=true) const override;

public:
  bool set(uint16_t vendorID,
           uint16_t typeID,
           const char* buf,
           size_t length);

protected:
  static string ipv4(const void* addr);
  static string ipv6(const void* addr);

private:
  PacketPtr packet_;
  timeval timestamp_;
};

} } } //namespace pe { namespace pcap { namespace parsers {

#endif // _PE_PCAP_PARSER_MESSAGE__H_20200719_