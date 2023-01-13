#ifndef _PE_LIVECAPTURE_SNIFFER__H_20200710_
#define _PE_LIVECAPTURE_SNIFFER__H_20200710_

#include "types.h"

#include <boost/asio/io_service.hpp>

#include "tools/signals.h"

namespace pe { namespace livecapture {

class Packet;

class Sniffer : noncopyable
{
public:
  using PacketPtr = std::shared_ptr<Packet>;
  using NotifySignal = signals::signal<void(const PacketPtr& p)>;
  using FailSignal = signals::signal<void(const boost::system::error_code& error)>;

public:
  virtual ~Sniffer()=default;

public:
  virtual boost::asio::io_service& ios()=0;
  virtual const string& name() const=0;

public:
  NotifySignal& notify() { return notify_; }
  FailSignal& fail() { return fail_; }

private:
  NotifySignal notify_;
  FailSignal fail_;
};

} } //namespace pe { namespace livecapture {

#endif //_PE_LIVECAPTURE_SNIFFER__H_20200710_
