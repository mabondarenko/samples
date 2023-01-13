#include "connection.h"

#include <boost/format.hpp>

#include "sockets/iconnection.h"
#include "tools/datetime.h"
#include "sniffer.h"
#include "trace.h"

namespace pe { namespace livecapture {

PE_DECLARE_LOG_CHANNEL(CONNECTION_LOG, "live.connection[");
#define THISLOG CONNECTION_LOG << id() << "]: "

Connection::Connection(const IConnectionPtr& connection,
                       const SnifferPtr& _sniffer,
                       const string& _filter,
                       Terminate* _terminate)
  : connection_(connection)
  , tmrCapture_(connection->ios())
  , sniffer_(_sniffer)
  , filter_(_filter)
  , rdbuf_(4 * 1024)
  , termination_(connect(_terminate))
{
  PE_DEBUG(THISLOG << "created...");

  onpacket_ = sniffer_->notify().connect([this](const PacketPtr& p) { post(p); });
}

Connection::~Connection()
{
  PE_DEBUG(THISLOG << "destroying...");

  onpacket_.disconnect();
  termination_.disconnect();
  terminate_();
}

boost::asio::io_service& Connection::ios()
{
  return connection_->ios();
}

uint64_t Connection::id() const
{
  return connection_->id();
}

signals::scoped Connection::connect(signals::simple& sig)
{
  return sig.connect_extended([this](const signals::connection& c){
    PE_DEBUG(THISLOG << "termination signal received!");
    c.disconnect();
    tmrCapture_.cancel();
    connection_->close();
  });
}

signals::scoped Connection::connect(signals::simple* sig)
{
  return sig ? connect(*sig) : signals::scoped();
}

void Connection::start(uint64_t seconds)
{
  PE_DEBUG(THISLOG << "starting capture for " << seconds << " seconds...");

  tmrCapture_.expires_from_now(boost::posix_time::seconds(seconds));

  auto self = shared_from_this();
  tmrCapture_.async_wait([self, this](const boost::system::error_code& error) {
    if (!error)
    {
      PE_DEBUG(THISLOG << "capture timer expired, closing connection...");
      connection_->close();
    }
  });

  read();
}

void Connection::read()
{
  auto self = shared_from_this();
  auto handler = [self, this](const boost::system::error_code& error, const size_t nread)
  {
    if (!error)
    {
      if (proccess(rdbuf_, nread))
        return read();
    }

    //free itself for closing...
    tmrCapture_.cancel();
  };

  connection_->read(rdbuf_, handler);
}

bool Connection::proccess(const buffer& buf, size_t nread)
{
  PE_UNUSED(buf);
  PE_UNUSED(nread);

  //PE_TRACE(THISLOG << nread << " bytes has been received.");
  return true;
}

void Connection::post(const PacketPtr& p)
{
  auto self = shared_from_this();
  auto handler = [this, self, p]()
  {
    if (p->data() && filter(*p->data()))
    {
      boost::format fmt("%1% %2%:%3% -> %4%:%5%\n");
      string hdr = (fmt % datetime::toISOString(p->timestamp())
                        % p->srcAddr()
                        % p->srcPort()
                        % p->dstAddr()
                        % p->dstPort()).str();

      if (auto msg = std::make_shared<sockets::Message>(hdr))
      {
        msg->push_back(p->data());
        //PE_TRACE(THISLOG << "writing " << data.length() << " bytes packet...");
        connection_->write(msg);
      }
    }
    else
    {
      //PE_TRACE(THISLOG << "packet has skipped by filter...");
    }
  };

  ios().post(handler);
}

bool Connection::filter(const string& p)
{
  if (filter_.empty() || p.empty())
    return false;

  if (filter_.length()==1 && filter_[0]=='*')
    return true;

  return p.find(filter_)!=string::npos;
}

} } //namespace pe { namespace livecapture {