#ifndef _PE_PCAP_GRABBER_DEVICE__H_20191108_
#define _PE_PCAP_GRABBER_DEVICE__H_20191108_

#include "types.h"

#include <pcap.h>

namespace pe { namespace pcap { namespace grabber {

class Device : noncopyable
{
public:
  struct Info
  {
    string name;
    string description;
  };

  using InfoList = std::vector<Info>;
  using Handle = pcap_t*;

public:
  Device(const string& interface,  //or filename --> offline
         const string& filter);
  ~Device();

public:
  string lasterror() const;
  operator Handle() const { return handle(); }
  Handle handle() const { return handle_; }
  bool offline() const { return offline_; }
  int setnonblock() const;

public:
  static InfoList list();

private:
  Handle open(const string& interface, const string& filter) noexcept(false);
  Handle load(const string& file, const string& filter) noexcept(false);
  void raise(const char* func, const char* text) const noexcept(false);

private:
  static bool isfile(const string& interface);

private:
  bool offline_;
  Handle handle_;
};

} } } //namespace pe { namespace pcap { namespace grabber {

#endif //_PE_PCAP_GRABBER_DEVICE__H_20191108_