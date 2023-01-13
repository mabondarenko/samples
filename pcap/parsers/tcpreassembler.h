#ifndef _PE_PCAP_PARSERS_TCPREASSEMBLER__H_20200131_
#define _PE_PCAP_PARSERS_TCPREASSEMBLER__H_20200131_

#include "raw.h"

namespace pcpp { class TcpReassembly; }

namespace pe { namespace pcap { namespace parsers {

class TCPReassembler : public Raw
{
  using _Base = Raw;

public:
  TCPReassembler(int datalink);
  virtual ~TCPReassembler() override;

public:
  virtual PacketsList packets(const char* data, size_t len, const timeval& ts) override;

private:
  using Reassembler = pcpp::TcpReassembly;
  Reassembler* create();

private:
  std::unique_ptr<Reassembler> reasm_;
  const char* data_ = nullptr;
  size_t len_ = 0;
  PacketInfo info_;
};

} } } //namespace pe { namespace pcap { namespace parsers {

#endif //#ifndef _PE_PCAP_PARSERS_TCPREASSEMBLER__H_20200131_