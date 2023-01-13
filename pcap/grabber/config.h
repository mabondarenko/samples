#ifndef _PE_PCAP_GRABBER_CONFIG__H_20191114_
#define _PE_PCAP_GRABBER_CONFIG__H_20191114_

#include "types.h"

#include <nlohmann/json.hpp>

namespace pe { namespace pcap { namespace grabber {

struct Config
{
  using json = nlohmann::json;

  struct Capture
  {
    bool enabled = true;
    string interface = "lo";
    string filter = "tcp dst port 5030";
    string parser = "voip";

    Capture() {}
    explicit Capture(const json& from);
  };

  std::map<string, Capture> captures;

  struct Listener
  {
    string interface;
    size_t limit = 10;
    uint16_t minport = 20000;
    uint16_t maxport = 40000;
    uint64_t timeout = 90;
    Listener() : Listener(json()) {}
    explicit Listener(const string& ip) : interface(ip) {}
    explicit Listener(const json& from) : Listener("127.0.0.1", from) {}
    explicit Listener(const string& ip, const json& from);

  } listener;

  Config() {}
  explicit Config(const string& ip) : listener(ip) {}
  explicit Config(const json& from);
  explicit Config(const string& ip, const json& from);
  void read(const json& from);
  const Capture* capture(const string& name) const;
};

} } } //namespace pe { namespace pcap { namespace grabber {

#endif //_PE_PCAP_GRABBER_CONFIG__H_20191114_