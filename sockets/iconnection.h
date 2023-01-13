#ifndef _PE_SOCKETS_ICONNECTION__H_20200312_
#define _PE_SOCKETS_ICONNECTION__H_20200312_

#include "defs.h"

#include <functional>
#include <boost/asio/io_service.hpp>

#include "message.h"

namespace pe { namespace sockets {

struct IConnection
{
  using CID = uint64_t;
  using ConnectCallback = std::function<void(const boost::system::error_code& error)>;
  using ReadCallback = std::function<void(const boost::system::error_code& error, size_t nread)>;
  using RecvCallback = std::function<void(const boost::system::error_code& error,
                                          size_t nread,
                                          const string& addr,
                                          uint16_t port)>;
  using WriteCallback = std::function<void(const boost::system::error_code& error, size_t nwritten)>;

  virtual ~IConnection()=default;

  virtual string addr() const=0;
  virtual uint16_t port() const=0;
  virtual string remoteAddr() const=0;
  virtual uint16_t remotePort() const=0;
  virtual boost::system::error_code close()=0;
  virtual boost::asio::io_service& ios()=0;
  virtual const CID& id() const=0;

  virtual void connect(const string& addr, uint16_t port, ConnectCallback&& callback)=0;
  virtual void read(buffer& dst, ReadCallback&& callback)=0;
  virtual void recv(buffer& dst, RecvCallback&& callback)=0;
  virtual void write(const MessagePtr& msg)=0;
  virtual void writeto(const MessagePtr& msg, const string& addr, uint16_t port) { msg->destination(addr, port); return write(msg); }

  void write(buffer&& data) { return write(std::make_shared<Message>(data)); }
  template <class Array>
  void write(const Array& data) { return write(std::make_shared<Message>(data)); }

  void writeto(buffer&& data, const string& addr, uint16_t port) { return writeto(std::make_shared<Message>(data), addr, port); }
  template <class Array>
  void writeto(const Array& data, const string& addr, uint16_t port) { return writeto(std::make_shared<Message>(data), addr, port); }
};

} } //namespace pe { namespace sockets {

#endif //#ifndef _PE_SOCKETS_ICONNECTION__H_20200312_
