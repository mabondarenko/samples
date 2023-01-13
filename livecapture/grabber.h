#ifndef _PE_LIVECAPTURE_GRABBER__H_20200710_
#define _PE_LIVECAPTURE_GRABBER__H_20200710_

#include "types.h"

#include <boost/asio/io_service.hpp>
#include <boost/thread/condition.hpp>

#include "tools/signals.h"
#include "mutex.h"

namespace pe { namespace livecapture {

class Sniffer;

class Grabber : public std::enable_shared_from_this<Grabber>
              , noncopyable
{
public:
  using Descriptor = std::pair<uint16_t, uint64_t>; //pair<port, timeout>
  using SnifferPtr = std::shared_ptr<Sniffer>;

public:
  Grabber(boost::asio::io_service& ios, const string& subscriberId);
  virtual ~Grabber();

public:
  boost::asio::io_service& ios() { return ios_; }
  Descriptor open(const string& filter,
                  unsigned timeout,
                  const string& transport,
                  const string& capture=string()) noexcept(false);
  SnifferPtr sniffer(const string& capture=string()) noexcept(false);
  const string& subscriberId() const { return subscriberId_; }

protected:
  virtual SnifferPtr createSniffer(const string& capture=string())=0;
  virtual bool isCaptureExists(const string& capture) const=0;
  virtual const string& listenInterface() const=0;
  virtual uint16_t listenMinPort() const=0;
  virtual uint16_t listenMaxPort() const=0;
  virtual size_t connectionsLimit() const=0;
  virtual uint64_t listenTimeout() const=0;

private:
  using SnifferRef = std::weak_ptr<Sniffer>;
  using SnifferMap = std::map<string, SnifferRef>;

private:
  static const string& cname(const string& capture);
  static void notfound(const string& capture) noexcept(false);
  void shutdown();

private:
  boost::asio::io_service& ios_;
  string subscriberId_;
  mutex_t mtxcaptures_;
  mutex_t mtxsniffers_;
  boost::condition condition_;
  signals::simple terminate_;
  SnifferMap sniffers_;
  size_t captures_ = 0;
};

} } //namespace pe { namespace livecapture {

#endif //_PE_LIVECAPTURE_GRABBER__H_20200710_