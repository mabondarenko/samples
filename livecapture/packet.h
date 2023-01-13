#ifndef _PE_LIVECAPTURE_PACKET__H_20200710_
#define _PE_LIVECAPTURE_PACKET__H_20200710_

#include "types.h"

namespace pe { namespace livecapture {

struct Packet
{
  using pointer_t = std::shared_ptr<Packet>;
  using data_t = string;
  using data_ptr = std::shared_ptr<data_t>;

  virtual ~Packet()=default;

  virtual int afamily() const=0;
  virtual int ipproto() const=0;
  virtual int protocol() const=0;
  virtual bool isIPv4() const=0;
  virtual bool isIPv6() const=0;
  virtual bool isUDP() const=0;
  virtual bool isTCP() const=0;
  virtual bool isSIP() const=0;
  virtual bool isRTP() const=0;
  virtual const data_ptr& data() const=0;
  virtual const data_ptr& compressed() const=0;
  virtual const string& srcAddr() const=0;
  virtual uint16_t srcPort() const=0;
  virtual const string& dstAddr() const=0;
  virtual uint16_t dstPort() const=0;
  virtual const timeval& timestamp() const=0;
  virtual const string& correlationID() const=0;
  virtual pointer_t clone(bool touch=true) const=0;
};

using PacketPtr = Packet::pointer_t;

} } //namespace pe { namespace livecapture {

#endif //_PE_LIVECAPTURE_PACKET__H_20200710_
