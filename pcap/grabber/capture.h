#ifndef _PE_PCAP_GRABBER_CAPTURE__H_20191108_
#define _PE_PCAP_GRABBER_CAPTURE__H_20191108_

#include "types.h"

#include <functional>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>

#include "device.h"

namespace pe { namespace pcap { namespace grabber {

class Capture : public Device
{
  using _Base = Device;

public:
  using error_t = boost::system::error_code;
  using callback_t = std::function<bool(const error_t& error,
                                        const char* data,
                                        size_t len,
                                        const timeval& ts)>;

public:
  Capture(boost::asio::io_service& ios,
          const string& name,
          const string& interface,
          const string& filter);
  ~Capture();

public:
  boost::asio::io_service& ios() { return ios_; }
  const string& name() const { return name_; }
  void read(const callback_t& cb);
  int datalink() const { return datalink_; }

protected:
  bool process(const error_t& error, const callback_t& cb);

private:
  using timer_t = boost::asio::deadline_timer;
  using Stream = boost::asio::posix::stream_descriptor;
  Stream* open(boost::asio::io_service& ios);

private:
  std::unique_ptr<Stream> stream_;
  int datalink_;
  string name_;
  boost::asio::io_service& ios_;
  std::unique_ptr<timer_t> timer_;
};

} } } //namespace pe { namespace pcap { namespace grabber {

#endif //_PE_PCAP_GRABBER_CAPTURE__H_20191108_