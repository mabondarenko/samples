#include "capture.h"

#include "trace.h"

namespace pe { namespace pcap { namespace grabber {

PE_DECLARE_LOG_CHANNEL(CAPTURE_LOG, "pcap.capture[");
#define THISLOG CAPTURE_LOG << name() << "]: "

Capture::Capture(boost::asio::io_service& _ios,
                 const string& _name,
                 const string& _interface,
                 const string& _filter)
  : _Base(_interface, _filter)
  , stream_(open(_ios))
  , datalink_(pcap_datalink(handle()))
  , name_(_name)
  , ios_(_ios)
{
  PE_DEBUG(THISLOG << "created on iterface '" << _interface << "'...");
}

Capture::~Capture()
{
  PE_DEBUG(THISLOG << "destroing...");

  //lets the library close fd:
  if (stream_)
    stream_->release();
}

Capture::Stream* Capture::open(boost::asio::io_service& ios)
{
  if (offline())
    return nullptr;

  return new Stream(ios, setnonblock());
}

void Capture::read(const callback_t& cb)
{
  auto handler = [this, cb](const error_t& error, size_t)
  {
    if (process(error, cb))
      read(cb);
  };

  if (stream_)
    stream_->async_read_some(boost::asio::null_buffers(), handler);
  else
  {
    if (!timer_)
      timer_.reset(new timer_t(ios()));

    timer_->expires_from_now(boost::posix_time::milliseconds(1000));
    timer_->async_wait([this, handler, cb](const error_t& error) {
      if (!error)
      {
        handler(error, 0);
        timer_.reset();
      }
    });
  }
}

bool Capture::process(const error_t& error, const callback_t& cb)
{
  if (error)
  {
    timeval ts;
    gettimeofday(&ts, nullptr);
    return cb(error, nullptr, 0, ts);
  }

  bool ret = true;
  do
  {
    using namespace boost::system;

    pcap_pkthdr* hdr = nullptr;
    const char* data = nullptr;

    switch(pcap_next_ex(handle(), &hdr, reinterpret_cast<const u_char**>(&data)))
    {
    case -2:
      PE_DEBUG(THISLOG << "EOF");
      cb(boost::asio::error::eof, nullptr, 0, hdr->ts);
      return false; //EOF

    case 0:
      return ret; //timeout/no-data available

    case 1:
      ret = cb(error, data, hdr->caplen, hdr->ts);
      break;

    default:
      PE_ERROR(THISLOG << "pcap_next_ex() failed with error: '" << lasterror() << "'");
      if (!hdr)
        return false;
      ret = cb(boost::asio::error::broken_pipe, data, hdr->caplen, hdr->ts);
      break;
    }
  }
  while (ret);

  return ret;
}

} } } //namespace pe { namespace pcap { namespace grabber {
