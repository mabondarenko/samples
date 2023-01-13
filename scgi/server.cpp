#include "server.h"

#include "connection.h"
#include "dispatcher.h"
#include "pool.h"
#include "parser.h"
#include "sockets/tcp/server.h"
#include "trace.h"

namespace pe { namespace scgi {

PE_DECLARE_LOG_CHANNEL(THISLOG, "scgi.server: ");

Server::Server(const DispatcherPtr& _dispatcher,
               const string& addr,
               uint16_t port,
               size_t maxSize)
  : dispatcher_(_dispatcher)
  , pool_(std::make_shared<Pool>(dispatcher_->ios(), maxSize))
  , listener_(std::make_shared<Listener>(dispatcher_->ios(),
                                         port,
                                         addr,
                                         static_cast<int>(maxSize)))
{
  PE_DEBUG(THISLOG << "created...");

  accept();
}

Server::~Server()
{
  PE_DEBUG(THISLOG << "destroying...");

  listener_->cancel();
  pool_->shutdown();
}

void Server::accept()
{
  auto onAccept = [this](const Listener::SocketPtr& s) -> bool
  {
    PE_DEBUG(THISLOG << "incoming connection from '"
                     << s->remote_endpoint().address().to_string()
                     << ":" << s->remote_endpoint().port() << "'...");

    auto connection = pool_->getConnection(s);
    if (connection)
      std::make_shared<Parser>(dispatcher_)->read(connection);
    else
    {
      PE_WARNING(THISLOG << "no more connections allowed");
    }

    accept();

    return true;
  };

  listener_->accept(onAccept);
}

} } //namespace pe { namespace scgi {
