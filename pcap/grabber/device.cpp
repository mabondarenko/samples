#include "device.h"

#include <boost/format.hpp>

namespace pe { namespace pcap { namespace grabber {

Device::Device(const string& interface, const string& filter)
  : offline_(isfile(interface))
  , handle_(offline() ? load(interface, filter) : open(interface, filter))
{
}

Device::~Device()
{
  pcap_close(handle());
}

bool Device::isfile(const string& interface)
{
  bool ret = interface.find_first_of("./\\")!=string::npos;
  return ret;
}

pcap_t* Device::open(const string& interface, const string& filter)
{
  char errbuf[PCAP_ERRBUF_SIZE];
  *errbuf = '\0';

  bpf_u_int32 net = 0;
  bpf_u_int32 netmask = 0;

  if (pcap_lookupnet(interface.c_str(), &net, &netmask, errbuf))
    raise("pcap_lookupnet", errbuf);

  auto h = pcap_open_live(interface.c_str(), 64*1024, 1, 20, errbuf);
  if (!h)
    raise("pcap_open_live", errbuf);

  bpf_program bpf;

  if (pcap_compile(h, &bpf, filter.c_str(), 1, netmask))
  {
    string text = pcap_geterr(h);
    pcap_close(h);
    raise("pcap_compile", text.c_str());
  }

  if (pcap_setfilter(h, &bpf))
  {
    string text = pcap_geterr(h);
    pcap_freecode(&bpf);
    pcap_close(h);
    raise("pcap_setfilter", text.c_str());
  }

  pcap_freecode(&bpf);
  return h;
}

pcap_t* Device::load(const string& file, const string& filter)
{
  char errbuf[PCAP_ERRBUF_SIZE];
  *errbuf = '\0';

  auto h = pcap_open_offline(file.c_str(), errbuf);
  if (!h)
    raise("pcap_open_offline", errbuf);

  bpf_program bpf;
  if (pcap_compile(h, &bpf, filter.c_str(), 1, 0))
  {
    string text = pcap_geterr(h);
    pcap_close(h);
    raise("pcap_compile", text.c_str());
  }

  if (pcap_setfilter(h, &bpf))
  {
    string text = pcap_geterr(h);
    pcap_freecode(&bpf);
    pcap_close(h);
    raise("pcap_setfilter", text.c_str());
  }

  pcap_freecode(&bpf);

  return h;
}

string Device::lasterror() const
{
  auto perr = pcap_geterr(handle());
  return perr ? perr : string();
}

void Device::raise(const char* func, const char* text) const
{
  boost::format fmt("%1%() failed with error '%2%'");
  throw std::runtime_error((fmt % func % text).str());
}

Device::InfoList Device::list()
{
  InfoList result;

  char errbuf[PCAP_ERRBUF_SIZE];
  pcap_if_t* alldevs;

  if (!pcap_findalldevs(&alldevs, errbuf))
  {
    for (auto dev = alldevs; dev; dev = dev->next)
    {
      Info di;
      di.name = dev->name;

      if (dev->description)
        di.description = dev->description;

      result.emplace_back(di);
    }

    pcap_freealldevs(alldevs);
  }

  return result;
}

int Device::setnonblock() const
{
  char errbuf[PCAP_ERRBUF_SIZE];
  *errbuf = '\0';

  if (pcap_setnonblock(handle(), 1, errbuf))
    raise("pcap_setnonblock", errbuf);

  int fd = pcap_get_selectable_fd(handle());

  if (fd==PCAP_ERROR)
    raise("pcap_get_selectable_fd", errbuf);

  return fd;
}

} } } //namespace pe { namespace pcap { namespace grabber {
