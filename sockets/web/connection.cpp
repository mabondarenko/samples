#include "connection.h"

#include "tools/str.h"
#include "trace.h"

namespace pe { namespace sockets { namespace web {

PE_DECLARE_LOG_CHANNEL(CONNECTION_LOG, "websocket.connection[");
#define THISLOG CONNECTION_LOG << id() << "]: "

Connection::Connection(boost::asio::io_service& _ios,
                       const SocketPtr& _socket)
  : _Base(_ios)
  , socket_(_socket)
{
  socket_->start();

  PE_DEBUG(THISLOG << "created...");
}

Connection::~Connection()
{
  PE_DEBUG(THISLOG << "destroying...");
}

boost::system::error_code Connection::close()
{
  boost::system::error_code ec;
  socket_->terminate(websocketpp::lib::error_code());
  return ec;
}

string Connection::remoteAddr() const
{
  auto endpoint = socket_->get_remote_endpoint();
  string_list_t parts;
  tools::str::split(parts, endpoint, [](const char c) { return c==':'; });
  return parts.size()>=1 ? parts[0] : endpoint;
}

uint16_t Connection::remotePort() const
{
  auto endpoint = socket_->get_remote_endpoint();
  string_list_t parts;
  tools::str::split(parts, endpoint, [](const char c) { return c==':'; });
  if (parts.size()==2)
    return std::atoi(parts[1].c_str());
  return 0;
}

void Connection::connect(const string& addr, uint16_t port, ConnectCallback&& callback)
{
  PE_UNUSED(addr);
  PE_UNUSED(port);
  PE_UNUSED(callback);

  throw exceptions::not_implemented("websocket::connect not implemented yet");
}

void Connection::read(ReadCallback&& handler)
{
  guard_t g(mtxhandlers_);
  onread_ = handler;

  setHandlers();
}

Connection::ReadCallback Connection::resetHandlers()
{
  guard_t g(mtxhandlers_);

  socket_->set_close_handler(websocketpp::close_handler());
  socket_->set_fail_handler(websocketpp::fail_handler());

  ReadCallback ret = onread_;
  onread_ = ReadCallback();
  return ret;
}

void Connection::setHandlers()
{
  if (handlersSet_)
    return;

  handlersSet_ = true;

  auto self = shared_from_this();

  auto hclose = [this, self](websocketpp::connection_hdl)
  {
    PE_DEBUG(THISLOG << "connection closed");

    auto hread = resetHandlers();

    if (hread)
      hread(boost::asio::error::eof, 0);
  };

  auto hfail = [this, self, hclose](websocketpp::connection_hdl h)
  {
    PE_ERROR(THISLOG << "connection failed");
    hclose(h);
  };

  socket_->set_close_handler(hclose);
  socket_->set_fail_handler(hfail);
}

void Connection::send(const Message& msg, WriteCallback&& handler)
{
  size_t nwritten = 0;
  for(const auto& b: msg)
  {
    auto sz = boost::asio::buffer_size(b);
    auto data = boost::asio::buffer_cast<const void*>(b);

    //PE_TRACE(THISLOG << "sending " << sz << " bytes message");

    //socket_->send(data, sz, websocketpp::frame::opcode::text);
    socket_->send(data, sz); //TODO: add opcode

    nwritten += sz;
  }

  if (handler)
    handler(boost::system::error_code(), nwritten);
}

void Connection::onWriteComplete(size_t nwritten)
{
  //PE_TRACE(THISLOG << "write complete, nwritten = " << nwritten);
  _Base::onWriteComplete(nwritten);
}

void Connection::onWriteError(const boost::system::error_code& error, size_t nwritten)
{
  PE_ERROR(THISLOG << "error writing to socket: " << error.message() << ", nwritten = " << nwritten);
  _Base::onWriteError(error, nwritten);
}

void Connection::read(buffer& dst, ReadCallback&& callback)
{
  PE_UNUSED(dst);

  auto self = shared_from_this();

  auto handler = [this, self, callback](const boost::system::error_code& error, size_t nread) {

    if (!error)
      onReadComplete(nread);
    else
      onReadError(error, nread);

    try
    {
      if (callback)
        callback(error, nread);
    }
    catch(const std::exception& e)
    {
      PE_ERROR(THISLOG << "exception in read callback: " << e.what());
    }
  };

  read(handler);
}

void Connection::onReadComplete(size_t nread)
{
  //PE_TRACE(THISLOG << "read complete, nread = " << nread);
  _Base::onReadComplete(nread);
}

void Connection::onReadError(const boost::system::error_code& error, size_t nread)
{
  if (error == boost::asio::error::eof)
  {
    PE_DEBUG(THISLOG << "EOF on socket... connection will be closed");
  }
  else if (error == boost::asio::error::operation_aborted)
  {
    PE_DEBUG(THISLOG << "operation canceled");
  }
  else
  {
    PE_ERROR(THISLOG << "error reading from socket: " << error.message() << ", nread = " << nread);
  }

  _Base::onReadError(error, nread);
}

} } } //namespace pe { namespace sockets { namespace web {
