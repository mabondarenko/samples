#ifndef _PE_PCAP_PARSERS_RAW__H_20191115_
#define _PE_PCAP_PARSERS_RAW__H_20191115_

#include "iparser.h"

namespace pe { namespace pcap { namespace parsers {

class Raw : public IParser
          , noncopyable
{
public:
  Raw(int datalink=0) : datalink_(datalink) {}

public:
  virtual int datalink() const override { return datalink_; }
  virtual PacketsList packets(const char* data, size_t len, const timeval& ts) override;
  virtual const char* payload(const char* data, size_t, PacketInfo&) override { return data; }

private:
  int datalink_;
};

} } } //namespace pe { namespace pcap { namespace parsers {

#endif //#ifndef _PE_PCAP_PARSERS_RAW__H_20191115_