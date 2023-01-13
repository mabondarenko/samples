#ifndef _PE_PCAP_PARSERS_FACTORY__H_20191113_
#define _PE_PCAP_PARSERS_FACTORY__H_20191113_

#include "types.h"

namespace pe { namespace pcap { namespace parsers {

class IParser;
using IParserPtr = std::shared_ptr<IParser>;

struct Factory
{
  static IParserPtr create(const string& type, int datalink);
};

} } } //namespace pe { namespace pcap { namespace parsers {

#endif //_PE_PCAP_PARSERS_FACTORY__H_20191113_