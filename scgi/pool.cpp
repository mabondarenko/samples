#include "pool.h"

#include "trace.h"

namespace pe { namespace scgi {

PE_DECLARE_LOG_CHANNEL(THISLOG, "scgi.pool: ");

Pool::Pool(boost::asio::io_service& _ios, size_t _maxSize)
  : ios_(_ios)
  , maxSize_(_maxSize)
{
  PE_DEBUG(THISLOG << "created");
}

Pool::~Pool()
{
  PE_DEBUG(THISLOG << "destroying...");
}

Pool::ConnectionPtr Pool::getConnection(const SocketPtr& socket)
{
  ConnectionPtr conection;

  if (fshutdown_)
    return conection;

  guard_t g(mtxConnections_);

  if (connections_.size()<maxSize_)
  {
    auto self = shared_from_this();
    conection = std::make_shared<Connection>(socket, [this, self](const Connection::CID& id) {
      freeConnection(id);
    });

    if (conection)
      connections_[conection->id()] = conection;
  }

  return conection;
}

bool Pool::freeConnection(const Connection::CID& id)
{
  if (fshutdown_)
    return false;

  guard_t g(mtxConnections_);

  auto connection = connections_.find(id);
  if (connection!=connections_.end())
  {
    connections_.erase(connection);
    return true;
  }

  return false;
}

void Pool::shutdown()
{
  if (fshutdown_.exchange(true))
    return;

  PE_DEBUG(THISLOG << "shutdowning...");

  guard_t g(mtxConnections_);

  for(const auto& i: connections_)
  {
    auto connection = i.second.lock();
    if (connection)
      connection->close();
  }
}

} } //namespace pe { namespace scgi {
