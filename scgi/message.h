#ifndef _PE_SCGI_MESSAGE__20190227_
#define _PE_SCGI_MESSAGE__20190227_

#include "types.h"

#include "sockets/message.h"

namespace pe { namespace scgi {

class Message : public sockets::Message
{
  using _Base = sockets::Message;

public:
  struct Header
  {
    static const uint32_t kMARKER_VALUE = 0x000000FB;
    static const uint32_t kMAX_MSG_SIZE = 1 * 1024 * 1024; //1MB

    uint32_t marker = kMARKER_VALUE;
    uint32_t size;

    Header() : size(0) {}
    Header(size_t sz) : size(static_cast<uint32_t>(sz)) {}

    bool valid() const { return marker==kMARKER_VALUE; }
    bool overflow() const { return size > kMAX_MSG_SIZE; }
  };

  static_assert(sizeof(Header)==8, "check aligment settings");

public:
  explicit Message(const string& data)
    : _Base(data)
  {
    push_front(header(data));
  }

  explicit Message(const buffer& data)
    : _Base(data)
  {
    push_front(header(data));
  }

  explicit Message(buffer&& data)
    : _Base(data)
  {
    push_front(header(data));
  }

private:
  template<class Array>
  static data_ptr header(const Array& data)
  {
    static_assert(sizeof(buffer::value_type)==1, "invalid buffer value type");

    Header hdr(data.size());
    const auto* p = reinterpret_cast<buffer::const_pointer>(&hdr);
    return data_ptr(new buffer(p, p + sizeof(hdr)));
  }
};

} } //namespace pe { namespace scgi {

#endif //#ifndef _PE_SCGI_MESSAGE__20190227_
