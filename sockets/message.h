#ifndef _PE_SOCKETS_MESSAGE__H_20191002_
#define _PE_SOCKETS_MESSAGE__H_20191002_

#include "types.h"

#include <boost/asio/buffer.hpp>

namespace pe { namespace sockets {

class Message
{
public:
  using data_t = string;
  using data_ptr = std::shared_ptr<data_t>;

public:
  explicit Message(const data_ptr& data)
    : data_{ data }
    , buffer_{ boost::asio::buffer(*data_.front()) }
  {
  }

  explicit Message(const string& data)
    : data_{ data_ptr(new data_t(data.begin(), data.end())) }
    , buffer_{ boost::asio::buffer(*data_.front()) }
  {
  }

  explicit Message(const string& data, size_t count)
    : data_{ data_ptr(new data_t(data.begin(), data.begin()+count)) }
    , buffer_{ boost::asio::buffer(*data_.front()) }
  {
  }

  explicit Message(const buffer& data)
    : data_{ data_ptr(new data_t(data.begin(), data.end())) }
    , buffer_{ boost::asio::buffer(*data_.front()) }
  {
  }

  explicit Message(const buffer& data, size_t count)
    : data_{ data_ptr(new data_t(data.begin(), data.begin()+count)) }
    , buffer_{ boost::asio::buffer(*data_.front()) }
  {
  }

  explicit Message(data_t&& data)
      : data_{ data_ptr(new data_t(data)) }
      , buffer_{ boost::asio::buffer(*data_.front()) }
  {
  }

  size_t dataSize() const
  {
    size_t sz = 0;
    for (const auto& d: data_)
      sz += d->size();
    return sz;
  }

  void destination(const string& addr, uint16_t port) { addr_ = addr; port_ = port; }
  const string& addr() const { return addr_; }
  uint32_t port() const { return port_; }

  template<class DataType>
  void push_front(const DataType& data) { return push_front(data_ptr(new data_t(data.begin(), data.end()))); }
  template<class DataType>
  void push_front(const DataType& data, size_t count) { return push_front(data_ptr(new data_t(data.begin(), data.begin()+count))); }
  void push_front(const data_ptr& data)
  {
    data_.push_back(data);
    buffer_.insert(buffer_.begin(), boost::asio::buffer(*data));
  }

  template<class DataType>
  void push_back(const DataType& data) { return push_back(data_ptr(new data_t(data.begin(), data.end()))); }
  template<class DataType>
  void push_back(const DataType& data, size_t count) { return push_back(data_ptr(new data_t(data.begin(), data.begin()+count))); }
  void push_back(const data_ptr& data)
  {
    data_.push_back(data);
    buffer_.push_back(boost::asio::buffer(*data));
  }

  // Implement the ConstBufferSequence requirements.
  using value_type = boost::asio::const_buffer;
  using sequence = std::vector<value_type>;
  using const_iterator = sequence::const_iterator;
  const_iterator begin() const { return buffer_.begin(); }
  const_iterator end() const { return buffer_.end(); }

private:
  std::vector<data_ptr> data_;
  std::vector<value_type> buffer_;
  string addr_;
  uint16_t port_;
};

using MessagePtr = std::shared_ptr<Message>;

} } //namespace pe { namespace sockets {

#endif //#ifndef _PE_SOCKETS_MESSAGE__H_20191002_
