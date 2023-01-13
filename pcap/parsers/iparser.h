#ifndef _PE_PCAP_PARSERS_IPARSER__H_20191113_
#define _PE_PCAP_PARSERS_IPARSER__H_20191113_

#include "types.h"

#include "packet.h"

namespace pe { namespace pcap { namespace parsers {

struct IParser
{
  using PacketsList = std::vector<PacketPtr>;

  virtual ~IParser()=default;
  virtual int datalink() const = 0;
  virtual PacketsList packets(const char* data, size_t len, const timeval& ts)=0;
  virtual const char* payload(const char* data, size_t len, PacketInfo& info)=0;
};

} } } //namespace pe { namespace pcap { namespace parsers {

#endif //_PE_PCAP_PARSERS_IPARSER__H_20191113_