#include "pool.h"

#include <boost/algorithm/string.hpp>

namespace pe { namespace redis {

Pool create_simple_pool(const string& addr)
{
  string_list_t parts;
  boost::split(parts, addr, [](char c) { return c==':'; });

  string host = "localhost";
  if (!parts.empty() && !parts[0].empty())
    host = parts[0];

  uint16_t port = 0;
  if (parts.size()>1)
    port = static_cast<uint16_t>(std::atoi(parts[1].c_str()));

  if (port==0)
    port = 6379;

  return redis3m::simple_pool::create(parts[0], port);
}

} } //namespace pe { namespace redis {
