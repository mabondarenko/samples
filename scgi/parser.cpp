#include "parser.h"

#include "connection.h"
#include "dispatcher.h"

#include "trace.h"

namespace pe { namespace scgi {

PE_DECLARE_LOG_CHANNEL(THISLOG, "scgi.parser: ");

Parser::Parser(const DispatcherPtr& _dispatcher)
  : dispatcher_(_dispatcher)
  , rdbuf_(8 * 1024)
{
  PE_DEBUG(THISLOG << "created...");
}

Parser::~Parser()
{
  PE_DEBUG(THISLOG << "destroying...");
}

void Parser::read(const ConnectionPtr& connection)
{
  auto self = shared_from_this();

  auto handler = [this, self, connection](const boost::system::error_code& error, const size_t nread) {

    if (!error)
    {
      if (process(connection, rdbuf_, nread))
        read(connection);
    }
  };

  connection->read(rdbuf_, handler);
}

bool Parser::process(const ConnectionPtr& connection, const buffer& buf, size_t size)
{
  if (failed_)
  {
    //connection MUST BE closed...
    PE_ERROR(THISLOG << "calling process() after fail on connnection " << connection->id());
    return false;
  }

  size_t i = 0;
  while(i<size)
  {
    if (nread_ < sizeof(header_))
    {
      //message header is not read completely yet
      auto p = reinterpret_cast<buffer::pointer>(&header_);
      p[nread_++] = buf[i++];
    }
    else
    {
      //message header is completely read:
      if (nread_ == sizeof(header_))
      {
        //check the header...
        if (!header_.valid())
        {
          //invalid marker in header
          PE_ERROR(THISLOG << "invalid message header/marker: " << header_.marker
                            << ". connection " << connection->id() << " will be closed.");

          connection->close();
          failed_ = true;
          return false;
        }

        if (header_.overflow())
        {
          //message size TOO LARGE
          skipmsg_ = true;
          PE_ERROR(THISLOG << "message size (" << header_.size << ") exceeds maximum ("
                            << header_.kMAX_MSG_SIZE
                            << "), we will ignore it... connection: "
                            << connection->id());
        }
        else
        {
          skipmsg_ = false;
          msgsize_ = 0;

          msgbuf_.resize(header_.size);

          if (header_.size == 0)
          {
            //someone forgot to check the length of the string before sending...
            //it's not a critical. hope the next message will be good

            PE_ERROR(THISLOG << "message with ZERO size, skip it... connection: " << connection->id());

            nread_ = 0;
            continue;
          }
        }
      }

      size_t needs = header_.size - msgsize_;
      if (needs > size - i)
        needs = size - i;

      if (!skipmsg_)
      {
        auto dst = msgbuf_.begin() + static_cast<ssize_t>(msgsize_);
        auto begin = buf.begin() + static_cast<ssize_t>(i);
        auto end = begin + static_cast<ssize_t>(needs);

        std::copy(begin, end, dst);
      }

      msgsize_ += needs;

      if (msgsize_ == header_.size)
      {
        if (!skipmsg_)
        {
          bool ret = false;

          try
          {
            ret = dispatcher_->onMessage(connection, msgbuf_);
          }
          catch(std::exception& e)
          {
            PE_ERROR(THISLOG << "dispatch message failed on connection "
                              << connection->id()
                              << ", error: " << e.what());
          }

          msgbuf_.clear();

          if (!ret)
          {
            PE_ERROR(THISLOG << "message dispatching/validation failed. connection "
                              << connection->id() << " will be closed.");
            connection->close();
            failed_ = true;
            return ret;
          }
        }
        else
          skipmsg_ = false;

        msgsize_ = 0;
        nread_ = 0;
      }
      else
        nread_ += needs;

      i += needs;
    }
  }

  return true;
}

} } //namespace pe { namespace scgi {
