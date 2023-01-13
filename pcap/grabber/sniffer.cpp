#include "sniffer.h"

#include "pcap/parsers/iparser.h"
#include "pcap/parsers/factory.h"
#include "pcap/parsers/message.h"
#include "capture.h"
#include "trace.h"

namespace pe { namespace pcap { namespace grabber {

PE_DECLARE_LOG_CHANNEL(SNIFFER_LOG, "pcap.sniffer[");
#define THISLOG SNIFFER_LOG << name() << "]: "

Sniffer::Sniffer(boost::asio::io_service& _ios, const Params& _p)
  : capture_(new Capture(_ios, _p.name, _p.interface, _p.filter))
  , parser_(parsers::Factory::create(_p.parser, capture_->datalink()))
{
  PE_DEBUG(THISLOG << "created...");

  read();
}

Sniffer::~Sniffer()
{
  PE_DEBUG(THISLOG << "destroing...");
}

boost::asio::io_service& Sniffer::ios()
{
  return capture_->ios();
}

const string& Sniffer::name() const
{
  return capture_->name();
}

void Sniffer::read()
{
  auto handler = [&](const Capture::error_t& error,
                     const char* data,
                     size_t len,
                     const timeval& ts)
  {
    if (!error)
      return process(data, len, ts);

    if (error == boost::asio::error::operation_aborted)
    {
      PE_DEBUG(THISLOG << "capture canceled");
    }
    else if (error == boost::asio::error::eof)
    {
      PE_DEBUG(THISLOG << error.message());
      fail()(error);
    }
    else
    {
      PE_ERROR(THISLOG << "capture error: " << error.message());
      fail()(error);
    }

    return false;
  };

  capture_->read(handler);
}

bool Sniffer::process(const char* data, size_t len, const timeval& ts)
{
  auto handler = [this](const parsers::PacketPtr& p)
  {
    try
    {
      notify()(std::make_shared<parsers::Message>(p));
    }
    catch(const std::exception& e)
    {
      PE_ERROR(THISLOG << "failed to process packet: " << e.what());
    }
  };

  try
  {
    for (const auto& p: parser_->packets(data, len, ts))
      handler(p);
  }
  catch(const std::exception& e)
  {
    PE_ERROR(THISLOG << "failed to parse packet: " << e.what());
  }

  return true;
}

} } } //namespace pe { namespace pcap { namespace grabber {
