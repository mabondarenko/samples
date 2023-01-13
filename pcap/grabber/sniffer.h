#ifndef _PE_PCAP_GRABBER_SNIFFER__H_20191002_
#define _PE_PCAP_GRABBER_SNIFFER__H_20191002_

#include "types.h"

#include <boost/asio/io_service.hpp>

#include "livecapture/sniffer.h"

namespace pe { namespace pcap {

namespace parsers { class IParser; }

namespace grabber {

class Capture;

class Sniffer : public livecapture::Sniffer
{
public:
  struct Params
  {
    string name;
    string interface;
    string filter;
    string parser;
  };

public:
  Sniffer(boost::asio::io_service& ios, const Params& params);
  virtual ~Sniffer() override;

public:
  virtual boost::asio::io_service& ios() override;
  virtual const string& name() const override;

protected:
  void read();
  bool process(const char* data, size_t len, const timeval& ts);

private:
  std::unique_ptr<Capture> capture_;
  std::shared_ptr<parsers::IParser> parser_;
};

} } } //namespace pe { namespace pcap { namespace grabber {

#endif //_PE_PCAP_GRABBER_SNIFFER__H_20191002_
