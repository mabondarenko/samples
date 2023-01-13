#include "voip.h"

#include <boost/format.hpp>
#include <sstream>
#include <fstream>
#include <snappy-c.h>

#include "tools/datetime.h"
#include "voipmonitor/pcap_queue_block.h"

#include "smart.h"

#include "trace.h"

namespace pe { namespace pcap { namespace parsers {

PE_DECLARE_LOG_CHANNEL(VOIP_LOG, "pcap.voipmon[");
#define THISLOG VOIP_LOG << stream() << "]: "

class VoIP::Reassembler
{
  using PacketsList = IParser::PacketsList;

public:
  Reassembler(const string& stream);
  ~Reassembler();

public:
  PacketsList process(const PacketPtr& p);
  const string& stream() const { return stream_; }
  void store();

private:
  static string decompress(const char* data,
                           size_t size,
                           size_t maxSize);

private:
  PacketsList parse(const char* payload, size_t size);
  PacketsList packets(const char* data,
                      size_t size,
                      const uint32_t* offsets,
                      uint32_t count,
                      int hm);
  const char* payload(const char* data,
                      size_t size,
                      int hm,
                      PacketInfo& info);
  void reset();

private:
  enum class State
  {
    kWAIT_HEADER,
    kREAD_HEADER,
    kREAD_BODY
  };

private:
  string stream_;
  string buffer_;
  size_t dataSize_ = 0;
  State state_ = State::kWAIT_HEADER;
};

VoIP::PacketsList VoIP::packets(const char* data, size_t len, const timeval& ts)
{
  PacketsList result;
  for (const auto& p: _Base::packets(data, len, ts))
  {
    auto res = reassembler(p)->process(p);
    result.insert(result.end(), res.begin(), res.end());
  }
  return result;
}

VoIP::ReassemblerPtr VoIP::reassembler(const PacketPtr& p)
{
  boost::format fmt("%1%:%2%-->%3%:%4%");

  const auto& pi = p->info();
  auto key = (fmt % pi.srcAddr % pi.srcPort % pi.dstAddr % pi.dstPort).str();

  auto it = reasms_.find(key);
  if (it!=reasms_.end())
    return it->second;

  auto res = std::make_shared<Reassembler>(key);
  reasms_[key] = res;
  return res;
}

VoIP::Reassembler::Reassembler(const string& _stream)
  : stream_(_stream)
{
  PE_DEBUG(THISLOG << "stream created...");
}

VoIP::Reassembler::~Reassembler()
{
  PE_DEBUG(THISLOG << "stream destroying...");
}

VoIP::Reassembler::PacketsList VoIP::Reassembler::process(const PacketPtr& p)
{
  PacketsList result;

  try
  {
    const auto& data = p->data();
    result = parse(data->data(), data->size());
  }
  catch(std::exception& e)
  {
    PE_ERROR(THISLOG << "failed to parse packet: " << e.what());
    reset();
  }

  if (!result.empty())
    reset();

  return result;
}

void VoIP::Reassembler::reset()
{
  state_ = State::kREAD_HEADER;
  buffer_.clear();
  dataSize_ = 0;
}

VoIP::Reassembler::PacketsList VoIP::Reassembler::parse(const char* payload, size_t size)
{
  using header_t = pcap_block_store::pcap_block_store_header;
  static const size_t hdrSize = sizeof(header_t);

  if (state_ == State::kWAIT_HEADER)
  {
    if (size < hdrSize)
      return PacketsList(); //skip this packet

    state_ = State::kREAD_HEADER;
    return parse(payload, size);
  }
  else if (state_ == State::kREAD_BODY)
  {
    buffer_.append(payload, size);
    if (buffer_.size() >= dataSize_)
    {
      state_ = State::kREAD_HEADER;
      return parse(buffer_.data(), buffer_.size());
    }
    return PacketsList();
  }

  const header_t* hdr = reinterpret_cast<const header_t*>(payload);
  if (memcmp(hdr->title, PCAP_BLOCK_STORE_HEADER_STRING, PCAP_BLOCK_STORE_HEADER_STRING_LEN)!=0)
    throw exceptions::runtime_error("bad header title");

  if (!hdr->count)
    throw exceptions::runtime_error("bad packet count");

  size_t dataSize = hdr->size_compress ? hdr->size_compress : hdr->size;

  if (!dataSize || hdr->size_compress > hdr->size)
    throw exceptions::runtime_error("bad data size");

  size_t offsetsSize = hdr->count * sizeof(uint32_t);

  if (hdrSize + offsetsSize + dataSize > size)
  {
    state_ = State::kREAD_BODY;
    dataSize_ = hdrSize + offsetsSize + dataSize;
    buffer_.append(payload, size);
    return PacketsList();
  }

  const uint32_t* offsets = reinterpret_cast<const uint32_t*>(hdr+1);

  uint32_t lastOffset = 0;
  for(uint32_t i=0; i<hdr->count; ++i)
  {
    if (lastOffset>offsets[i])
      throw exceptions::runtime_error("bad offsets");

    lastOffset = offsets[i];
  }

  if (lastOffset>=hdr->size)
    throw exceptions::runtime_error("bad last offset/data size");

  const char* data = payload + hdrSize + offsetsSize;

  if (dataSize < hdr->size)
  {
    string tmpbuf = decompress(data, dataSize, hdr->size);
    return packets(tmpbuf.c_str(), tmpbuf.size(), offsets, hdr->count, hdr->hm);
  }

  return packets(data, dataSize, offsets, hdr->count, hdr->hm);
}

VoIP::Reassembler::PacketsList VoIP::Reassembler::packets(const char* data,
                                                          size_t size,
                                                          const uint32_t* offsets,
                                                          uint32_t count,
                                                          int hm)
{
  auto packetSize = [&](uint32_t index) -> size_t {
    if (index==count-1)
      return size - offsets[index];
    else if (index<count)
      return offsets[index+1] - offsets[index];

    return 0;
  };

  PacketsList result;
  for(uint32_t i=0; i<count; ++i)
  {
    const char* pkt = data + offsets[i];
    auto pktSize = packetSize(i);

    if (pkt + pktSize <= data + size)
    {
      PacketInfo info;
      auto p = payload(pkt, pktSize, hm, info);
      if (p < pkt + pktSize)
        result.push_back(std::make_shared<Packet>(p, pkt + pktSize - p, info));
    }
  }

  return result;
}

const char* VoIP::Reassembler::payload(const char* data,
                                       size_t size,
                                       int hm,
                                       PacketInfo& info)
{
  auto hsize = (hm==pcap_block_store::plus2 ? sizeof(pcap_pkthdr_plus2) : sizeof(pcap_pkthdr_plus));

  if (hsize<size)
  {
    const pcap_pkthdr_plus* hdr = reinterpret_cast<const pcap_pkthdr_plus*>(data);
    size_t caplen = hdr->header_fix_size.caplen;

    if (caplen < size && hdr->header_ip_offset<size)
    {
      hsize = size - caplen; //aligment bug workaround
      auto* cap = data + hsize + hdr->header_ip_offset;

      if (cap < data + size)
      {
        info.ts.tv_sec = hdr->header_fix_size.ts_tv_sec;
        info.ts.tv_usec = hdr->header_fix_size.ts_tv_usec;
        return Smart(DLT_RAW).payload(cap, caplen - hdr->header_ip_offset, info);
      }
    }
  }

  return data + size;
}

string VoIP::Reassembler::decompress(const char* data, size_t size, size_t maxSize)
{
  size_t outbufSize = maxSize + 1;

  string result(outbufSize, 0);

	snappy_status status = snappy_uncompress(data, size, (char*)result.data(), &outbufSize);

	switch(status)
  {
  case SNAPPY_OK:
    if (outbufSize!=maxSize)
      throw exceptions::runtime_error("snappy_uncompress: bad uncompressed size");

    result.resize(outbufSize);
		break;

	case SNAPPY_INVALID_INPUT:
    throw exceptions::runtime_error("snappy_uncompress: invalid input");

	case SNAPPY_BUFFER_TOO_SMALL:
    throw exceptions::runtime_error("snappy_uncompress: buffer is too small");

	default:
    throw exceptions::runtime_error("snappy_uncompress: unknown error");
	}

  return result;
}

} } } //namespace pe { namespace pcap { namespace parsers {
