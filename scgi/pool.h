#ifndef _PE_SCGI_POOL__20190228_
#define _PE_SCGI_POOL__20190228_

#include "types.h"

#include <boost/asio/io_service.hpp>

#include "connection.h"
#include "mutex.h"

namespace pe { namespace scgi {

class Pool : public std::enable_shared_from_this<Pool>
           , noncopyable
{
  using SocketPtr = Connection::SocketPtr;
  using ConnectionPtr = std::shared_ptr<Connection>;

public:
  Pool(boost::asio::io_service& ios, size_t maxSize=200);
  ~Pool();

public:
  ConnectionPtr getConnection(const SocketPtr& socket);
  bool freeConnection(const Connection::CID& id);
  void shutdown();

public:
  boost::asio::io_service& ios() { return ios_; }
  size_t maxSize() const { return maxSize_; }

private:
  using ConnectionWeakPtr = std::weak_ptr<Connection>;
  using ConnectionMap = std::map<Connection::CID, ConnectionWeakPtr>;

private:
  boost::asio::io_service& ios_;
  size_t maxSize_;
  ConnectionMap connections_;
  mutex_t mtxConnections_;
  std::atomic_bool fshutdown_{false};
};

} } //namespace pe { namespace scgi {

#endif //#ifndef _PE_SCGI_POOL__20190228_
