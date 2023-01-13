#include "message.h"

#include <arpa/inet.h>
#include <sys/time.h>

namespace pe { namespace pcap { namespace parsers {

Message::Message(const PacketPtr& packet)
  : Message(packet, packet->info().ts)
{
}

Message::Message(const PacketPtr& packet,
                 const timeval& ts)
  : packet_(packet)
  , timestamp_(ts)
{
}

int Message::afamily() const
{
  if (isIPv4())
    return AF_INET;

  if (isIPv6())
    return AF_INET6;

  return AF_UNSPEC;
}

int Message::ipproto() const
{
  if (isUDP())
    return IPPROTO_UDP;

  if (isTCP())
    return IPPROTO_TCP;

  return IPPROTO_IP;
}

Message::pointer_t Message::clone(bool touch) const
{
  timeval ts;
  if (touch)
    gettimeofday(&ts, nullptr);

  pointer_t msg(new Message(packet_, touch ? ts : timestamp_));

  return msg;
}

} } } //namespace pe { namespace pcap { namespace parsers {