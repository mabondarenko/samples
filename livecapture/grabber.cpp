#include "grabber.h"

#include "srvfactory.h"
#include "sniffer.h"
#include "trace.h"

namespace pe { namespace livecapture {

PE_DECLARE_LOG_CHANNEL(THISLOG, "live.grabber: ");

Grabber::Grabber(boost::asio::io_service& _ios,
                 const string& _subscriberId)
  : ios_(_ios)
  , subscriberId_(_subscriberId)
{
}

Grabber::~Grabber()
{
  terminate_();

  shutdown();
}

const string& Grabber::cname(const string& capture)
{
  static const string def("default");
  return capture.empty() ? def : capture;
}

void Grabber::notfound(const string& capture)
{
  throw exceptions::item_not_found("capture '" + capture + "' not found");
}

Grabber::Descriptor Grabber::open(const string& filter,
                                  unsigned timeout,
                                  const string& transport,
                                  const string& capture)
{
  auto sname = cname(capture);

  PE_DEBUG(THISLOG << "trying to open server with '" << sname << "' capture...");

  if (!isCaptureExists(sname))
    notfound(sname);

  std::shared_ptr<Server> srv;
  {
    Server::Params params
    {
      listenInterface(),
      listenMinPort(),
      listenMaxPort(),
      { filter, timeout, sname }
    };

    guard_t g(mtxcaptures_);
    if (captures_ >= connectionsLimit())
      throw exceptions::too_many_items("limit of connections reached");

    PE_DEBUG(THISLOG << "opening new capture server. currently " << captures_ << " servers is open");

    srv = ServerFactory::create(transport, shared_from_this(), params, &terminate_);
    if (!srv)
      throw exceptions::runtime_error("failed to create '" + transport + "' server");

    ++captures_;
  }

  auto onterminate = [&]()
  {
    guard_t g(mtxcaptures_);

    if (--captures_==0)
      condition_.notify_one();
  };

  srv->onterminate().connect(onterminate);
  srv->start(listenTimeout());

  return Descriptor(srv->port(), listenTimeout());
}

Grabber::SnifferPtr Grabber::sniffer(const string& capture)
{
  auto sname = cname(capture);

  guard_t g(mtxsniffers_);

  auto& ref = sniffers_[sname];

  auto ptr = ref.lock();
  if (!ptr)
  {
    ptr = createSniffer(sname);
    ref = ptr;
  }

  //cleanup map from outdated ptrs
  for (auto s=sniffers_.begin(); s!=sniffers_.end(); )
  {
    if (!s->second.lock())
      s = sniffers_.erase(s);
    else
      ++s;
  }

  if (!ptr)
    throw exceptions::runtime_error("failed to create capture '" + sname + "'.");
  else
  {
    PE_DEBUG(THISLOG << "capture '" << sname << "' successfully requested...");
  }

  return ptr;
}

void Grabber::shutdown()
{
  guard_t g(mtxcaptures_);

  while(captures_)
    condition_.wait(g);
}

} } //namespace pe { namespace livecapture {
