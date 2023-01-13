#ifndef _PE_PCAP_PARSERS_VOIP__H_20191113_
#define _PE_PCAP_PARSERS_VOIP__H_20191113_

#include "tcpreassembler.h"

namespace pe { namespace pcap { namespace parsers {

class VoIP : public TCPReassembler
{
  using _Base = TCPReassembler;

public:
  VoIP(int datalink) : _Base(datalink) {}

public:
  virtual PacketsList packets(const char* data, size_t len, const timeval& ts) override;

private:
  class Reassembler;
  using ReassemblerPtr = std::shared_ptr<Reassembler>;
  using ReassemblersMap = std::map<string, ReassemblerPtr>;

private:
  ReassemblerPtr reassembler(const PacketPtr& p);

private:
  ReassemblersMap reasms_;
};

} } } //namespace pe { namespace pcap { namespace parsers {

#endif //#ifndef _PE_PCAP_PARSERS_VOIP__H_20191113_