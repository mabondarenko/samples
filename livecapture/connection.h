#ifndef _PE_LIVECAPTURE_CONNECTION__H_20200710_
#define _PE_LIVECAPTURE_CONNECTION__H_20200710_

#include "types.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>

#include "tools/signals.h"
#include "packet.h"

namespace pe { namespace sockets { class IConnection; } }

namespace pe { namespace livecapture {

class Sniffer;

class Connection : public std::enable_shared_from_this<Connection>
                 , noncopyable
{
  using IConnectionPtr = std::shared_ptr<sockets::IConnection>;
  using SnifferPtr = std::shared_ptr<Sniffer>;
  using Terminate = signals::simple;

public:
  Connection(const IConnectionPtr& connection,
             const SnifferPtr& sniffer,
             const string& filter,
             Terminate* terminate=nullptr);
  virtual ~Connection();

public:
  Terminate& onterminate() { return terminate_; }

public:
  uint64_t id() const;
  boost::asio::io_service& ios();
  void start(uint64_t period=60);

protected:
  bool filter(const string& p);
  void read();
  bool proccess(const buffer& buf, size_t nread);
  signals::scoped connect(signals::simple& sig);
  signals::scoped connect(signals::simple* sig);
  void post(const PacketPtr& p);

private:
  using timer_t = boost::asio::deadline_timer;

private:
  IConnectionPtr connection_;
  timer_t tmrCapture_;
  SnifferPtr sniffer_;
  string filter_;
  buffer rdbuf_;
  Terminate terminate_;
  signals::scoped termination_;
  signals::scoped onpacket_;
};

} } //namespace pe { namespace livecapture {

#endif //_PE_LIVECAPTURE_CONNECTION__H_20200710_