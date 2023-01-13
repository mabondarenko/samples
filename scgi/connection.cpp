#include "connection.h"

#include "trace.h"

namespace pe { namespace scgi {

PE_DECLARE_LOG_CHANNEL(CONNECTION_LOG, "scgi.connection[");
#define THISLOG CONNECTION_LOG << id() << "]: "

Connection::Connection(const SocketPtr& _socket, const DestroyCallback& _callback)
  : _Base(_socket)
  , callback_(_callback)
{
  PE_DEBUG(THISLOG << "created...");
}

Connection::~Connection()
{
  PE_DEBUG(THISLOG << "destroying...");

  if (callback_)
    callback_(id());
}

} } //namespace pe { namespace scgi {
