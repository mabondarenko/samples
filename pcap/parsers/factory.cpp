#include "factory.h"

#include "ip.h"
#include "raw.h"
#include "tcp.h"
#include "tcpreassembler.h"
#include "udp.h"
#include "voip.h"

namespace pe { namespace pcap { namespace parsers {

template<class P>
class ParserMap
{
public:
  IParserPtr get(int datalink)
  {
    auto& ptr = m_[datalink];
    if (!ptr)
      ptr = std::make_shared<P>(datalink);
    return ptr;
  }

private:
  std::map<int, IParserPtr> m_;
};

IParserPtr Factory::create(const string& type, int datalink)
{
  if (type=="ip")
  {
    static ParserMap<IP> p;
    return p.get(datalink);
  }
  else if (type=="tcp")
  {
    static ParserMap<TCP> p;
    return p.get(datalink);
  }
  else if (type=="udp")
  {
    static ParserMap<UDP> p;
    return p.get(datalink);
  }
  else if (type=="tcpstream")
  {
    return std::make_shared<TCPReassembler>(datalink); //context-depended
  }
  else if (type=="voip")
  {
    return std::make_shared<VoIP>(datalink); //context-depended
  }

  static const IParserPtr raw = std::make_shared<Raw>();
  return raw;
}

} } } //namespace pe { namespace pcap { namespace parsers {