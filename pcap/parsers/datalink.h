#ifndef _PE_PCAP_PARSERS_DATALINK__H_20191115_
#define _PE_PCAP_PARSERS_DATALINK__H_20191115_

#include "raw.h"

namespace pe { namespace pcap { namespace parsers {

class Datalink : public Raw
{
  using _Base = Raw;

public:
  Datalink(int datalink) : _Base(datalink) {}

public:
  virtual const char* payload(const char* data, size_t len, PacketInfo& info) override;

private:
  size_t headerSize(size_t len) const;
};

} } } //namespace pe { namespace pcap { namespace parsers {

#endif //#ifndef _PE_PCAP_PARSERS_DATALINK__H_20191115_