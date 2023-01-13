#ifndef _PE_SOCKETS_WEB_WSDEFS__H_20200313_
#define _PE_SOCKETS_WEB_WSDEFS__H_20200313_

#include "sockets/defs.h"

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

namespace pe { namespace sockets { namespace web {

using Acceptor = websocketpp::server<websocketpp::config::asio>;
using ConnectionPtr = Acceptor::connection_ptr;

} } } //namespace pe { namespace sockets { namespace web {

#endif //_PE_SOCKETS_WEB_WSDEFS__H_20200313_