#ifndef _PE_PCAP_TOOLS_OSTREAM__H_20200723_
#define _PE_PCAP_TOOLS_OSTREAM__H_20200723_

#include "types.h"

#include <ctime>
#include <ostream>

namespace pe { namespace pcap { namespace tools {

class OStream : noncopyable
{
public:
  OStream(std::ostream& out, uint32_t datalink=0);      //DLT_NULL
  //OStream(std::ostream& out, uint32_t datalink=1);    //DLT_EN10MB
  //OStream(std::ostream& out, uint32_t datalink=12);   //DLT_RAW
  //OStream(std::ostream& out, uint32_t datalink=108);  //DLT_LOOP
  ~OStream();

public:
  uint32_t datalink() const { return datalink_; }
  std::ostream& sout() { return out_; }
  std::time_t created() const { return created_; }
  std::time_t modified() const { return modified_; }
  size_t total() const { return total_; }

public:
  void write(const timeval& ts, //timestamp of packet
             int af,            //address family IPv4 or IPv6
             int proto,         //proto: IP, UDP, TCP
             const string& srcAddr,
             uint16_t srcPort,
             const string& dstAddr,
             uint16_t dstPort,
             const string& data);

private:
  std::ostream& init(std::ostream& out, uint32_t datalink);
  void touch() { modified_ = std::time(nullptr); }

private:
  static uint32_t testDataLink(uint32_t datalink);

private:
  uint32_t datalink_;
  size_t total_ = 0;
  std::ostream& out_;
  std::time_t created_ { std::time(nullptr) };
  std::time_t modified_ { created_ };
};

} } } //namespace pe { namespace pcap { namespace tools {

#endif //_PE_PCAP_TOOLS_OSTREAM__H_20200723_