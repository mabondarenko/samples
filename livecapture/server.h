#ifndef _PE_LIVECAPTURE_SERVER__H_20200710_
#define _PE_LIVECAPTURE_SERVER__H_20200710_

#include "types.h"

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>

#include "tools/signals.h"
#include "mutex.h"

namespace pe { namespace livecapture {

class Grabber;

class Server : public std::enable_shared_from_this<Server>
             , noncopyable
{
public:
  using GrabberPtr = std::shared_ptr<Grabber>;
  using Terminate = signals::simple;

  struct Params
  {
    string interface;
    uint16_t minport;
    uint16_t maxport;
    struct Capture
    {
      string filter;
      uint64_t period;
      string name;
    } capture;
  };

public:
  Server(const GrabberPtr& grabber,
         const Params& params,
         Terminate* terminate=nullptr);
  virtual ~Server();

public:
  GrabberPtr grabber() const { return grabber_.lock(); }
  Terminate& onterminate() { return terminate_; }
  const Params::Capture& params() const { return params_; }
  void start(uint64_t seconds);

public:
  virtual uint16_t port() const=0;

protected:
  virtual void accept()=0;
  virtual void cancel()=0;

protected:
  void wait(uint64_t seconds);
  void stopAcceptTimer();
  signals::scoped connect(signals::simple& sig);
  signals::scoped connect(signals::simple* sig);
  void connectOnFinish(signals::simple& sig);

private:
  using timer_t = boost::asio::deadline_timer;
  using GrabberRef = std::weak_ptr<Grabber>;

private:
  GrabberRef grabber_;
  std::unique_ptr<timer_t> tmrAccept_;
  mutex_t mtx_;
  Terminate terminate_;
  Params::Capture params_;
  signals::scoped termination_;
  signals::scoped finish_;
};

} } //namespace pe { namespace livecapture {

#endif //_PE_LIVECAPTURE_SERVER__H_20200710_