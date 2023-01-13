#include "ostream.h"

#include <cstring>
#include <pcap.h>

#include "ippacket.h"
#include "trace.h"

namespace pe { namespace pcap { namespace tools {

PE_DECLARE_LOG_CHANNEL(THISLOG, "pcap.ostream: ");

OStream::OStream(std::ostream& _out, uint32_t _datalink)
  : datalink_(testDataLink(_datalink))
  , out_(init(_out, _datalink))
{
}

OStream::~OStream()
{
}

std::ostream& OStream::init(std::ostream& out, uint32_t datalink)
{
  pcap_file_header hdr[1];
  memset(hdr, 0, sizeof(hdr));

  hdr->magic = 0xa1b2c3d4;
  hdr->version_major = 2;
	hdr->version_minor = 4;
	hdr->thiszone = 0;
	hdr->sigfigs = 0;
	hdr->snaplen = 0xFFFF;
  hdr->linktype = datalink;

  if (!out.write(reinterpret_cast<const char*>(hdr), sizeof(hdr)).good())
    throw exceptions::runtime_error("pcap.ostream: init failed");

  total_ += sizeof(hdr);

  return out;
}

void OStream::write(const timeval& ts, //timestamp of packet
                    int af, //address family IPv4 or IPv6
                    int proto, //proto: IP, UDP, TCP
                    const string& srcAddr,
                    uint16_t srcPort,
                    const string& dstAddr,
                    uint16_t dstPort,
                    const string& data)
{
  IPPacket packet(datalink(), af, proto, srcAddr, srcPort, dstAddr, dstPort, data.data(), data.size());

  struct pcaprec_hdr_s
  {
    uint32_t ts_sec;
    uint32_t ts_usec;
    uint32_t len;
    uint32_t caplen;

  } hdr[1];

  hdr->ts_sec = ts.tv_sec;
  hdr->ts_usec = ts.tv_usec;
  hdr->len = static_cast<bpf_u_int32>(packet.size());
  hdr->caplen = hdr->len;

  if (!sout().write(reinterpret_cast<const char*>(hdr), sizeof(hdr)).good())
    throw exceptions::runtime_error("pcap.ostream: failed to write packet header");

  total_ += sizeof(hdr);

  sout() << packet;

  if (!sout().good())
    throw exceptions::runtime_error("pcap.ostream: failed to write packet data");

  total_ += hdr->len;
  touch();
}

uint32_t OStream::testDataLink(uint32_t datalink)
{
  try
  {
    IPPacket::dataLinkSize(datalink);
  }
  catch(const std::exception& e)
  {
    throw exceptions::runtime_error("pcap.ostream: unsupported datalink=" + std::to_string(datalink));
  }
  return datalink;
}

} } } //namespace pe { namespace pcap { namespace tools {