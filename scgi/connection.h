#ifndef _PE_SCGI_CONNECTION__20190227_
#define _PE_SCGI_CONNECTION__20190227_

#include "types.h"

#include "sockets/tcp/connection.h"

#include "message.h"

namespace pe { namespace scgi {

class Connection : public sockets::tcp::Connection
{
  using _Base = sockets::tcp::Connection;
  using DestroyCallback = std::function<void(const CID& id)>;

public:
  Connection(const SocketPtr& socket, const DestroyCallback& callback);
  virtual ~Connection() override;

private:
  DestroyCallback callback_;
};

} } //namespace pe { namespace scgi {

#endif //#ifndef _PE_SCGI_CONNECTION__20190227_
