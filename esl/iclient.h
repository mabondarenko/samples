#ifndef _PE_ESL_ICLIENT__H_20191228_
#define _PE_ESL_ICLIENT__H_20191228_

#include "types.h"

#include <boost/asio/io_service.hpp>

#include "tools/signals.h"

namespace pe { namespace esl {

class IEvent;
class Command;

struct IClient
{
  using pointer_t = std::shared_ptr<IClient>;
  using ConnectSignal = signals::signal<void(const pointer_t&)>;

  using EventPtr = std::shared_ptr<IEvent>;
  using EventSignal = signals::signal<void(const pointer_t&, const EventPtr&)>;

  static pointer_t create(boost::asio::io_service& ios,
                          const string& password=string("ClueCon"),
                          const string& host=string("127.0.0.1"),
                          uint16_t port=8021);

  virtual boost::asio::io_service& ios()=0;
  virtual const string& host() const=0;
  virtual const string& password() const=0;
  virtual uint16_t port() const=0;

  virtual const string_set_t& eventMask() const=0;
  virtual void eventMask(const string_set_t& value, bool append=true)=0;

  virtual EventPtr send(const Command& cmd, u_int32_t timeoutms=3000)=0;
  virtual EventPtr send(const string& cmd, u_int32_t timeoutms=3000)=0;
  virtual EventPtr send(const char* cmd, u_int32_t timeoutms=3000)=0;

  virtual void start(const string_set_t& eventMask=string_set_t())=0;
  virtual void stop()=0;

  virtual ConnectSignal& onconnect()=0;
  virtual ConnectSignal& ondisconnect()=0;
  virtual EventSignal& onevent(const string& evtname)=0;

  virtual ~IClient()=default;
};

} } //namespace pe { namespace esl {

#endif //_PE_ESL_ICLIENT__H_20191228_