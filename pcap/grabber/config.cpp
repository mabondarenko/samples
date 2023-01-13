#include "config.h"

#include "tools/jsonhelper.h"

namespace pe { namespace pcap { namespace grabber {

Config::Capture::Capture(const json& from)
{
  enabled = jhlp::getValue(from, "enabled", enabled);
  interface = jhlp::getValue(from, "interface", interface);
  filter = jhlp::getValue(from, "filter", filter);
  parser = jhlp::getValue(from, "parser", parser);
}

Config::Listener::Listener(const string& ip, const json& from)
  : interface(ip)
{
  limit = jhlp::getValue(from, "limit", limit);
  interface = jhlp::getValue(from, "interface", interface);
  minport = jhlp::getValue(from, "minport", minport);
  maxport = jhlp::getValue(from, "maxport", maxport);
  timeout = jhlp::getValue(from, "timeout", timeout);
}

Config::Config(const json& from)
  : listener(jhlp::getValue<json>(from, "listener"))
{
  read(from);
}

Config::Config(const string& ip, const json& from)
  : listener(ip, jhlp::getValue<json>(from, "listener"))
{
  read(from);
}

void Config::read(const json& from)
{
  auto ii = jhlp::getValue<json>(from, "captures");

  for(auto i=ii.begin(); i!=ii.end(); ++i)
  {
    try
    {
      captures.emplace(i.key(), Capture(i.value()));
    }
    catch(const std::exception&)
    {
      //BAD value (not json)
    }
  }
}

const Config::Capture* Config::capture(const string& name) const
{
  auto it = captures.find(name);
  if (it!=captures.end())
    return &it->second;

  if (name.empty() || name=="default")
  {
    static const Capture def;
    return &def;
  }

  return nullptr;
}

} } } //namespace pe { namespace pcap { namespace grabber {