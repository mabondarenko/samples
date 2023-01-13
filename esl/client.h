#ifndef _PE_ESL_CLIENT__H_20191228_
#define _PE_ESL_CLIENT__H_20191228_

#include "iclient.h"

#include <atomic>
#include <freeswitch/esl/esl.h>

#include "mutex.h"

namespace pe { namespace esl {

class Client : public IClient
             , public std::enable_shared_from_this<Client>
             , noncopyable
{
  using handle_t = esl_handle_t*;

public:
  Client(boost::asio::io_service& ios,
         const string& password=string("ClueCon"),
         const string& host=string("127.0.0.1"),
         uint16_t port=8021);
  virtual ~Client() override;

public:
  virtual boost::asio::io_service& ios() override { return ios_; }
  virtual const string& host() const override { return host_; }
  virtual const string& password() const override { return password_; }
  virtual uint16_t port() const override { return port_; }

public:
  virtual const string_set_t& eventMask() const override { return eventmask_; }
  virtual void eventMask(const string_set_t& value, bool append=true) override;

  virtual EventPtr send(const Command& cmd, u_int32_t timeoutms=3000) override;
  virtual EventPtr send(const string& cmd, u_int32_t timeoutms=3000) override;
  virtual EventPtr send(const char* cmd, u_int32_t timeoutms=3000) override;

  virtual void start(const string_set_t& eventMask=string_set_t()) override;
  virtual void stop() override;

  virtual ConnectSignal& onconnect() override { return fonconnect_; }
  virtual ConnectSignal& ondisconnect() override { return fondisconnect_; }
  virtual EventSignal& onevent(const string& evtname) override;

protected:
  esl_handle_t* handle() { return &handle_; }
  virtual void onConnected();
  virtual void onDisconnected();

private:
  string eventList() const;
  bool connect(uint32_t timeout=1000);
  bool disconnect();
  bool dispatch(const EventPtr& evt);
  EventPtr parse(esl_event_t* evt);
  bool process(esl_event_t* evt);
  void poll();
  void recv();
  void reset();
  bool stopped() { return stopped_; }
  void subscribe();
  EventSignal* eventSignal(const string& evtname);

private:
  using EventSignalMap = std::map<string, EventSignal>;

private:
  boost::asio::io_service& ios_;
  string host_;
  string password_;
  uint16_t port_;
  esl_handle_t handle_;
  std::atomic_bool stopped_{ true };
  mutex_t mtxstop_;
  mutex_t mtxevents_;
  mutex_t mtxsignals_;
  mutex_t mtxsend_;
  condition_t cndstop_;
  string_set_t eventmask_;
  ConnectSignal fonconnect_;
  ConnectSignal fondisconnect_;
  EventSignalMap feventmap_;
};

} } //namespace pe { namespace esl {

#endif //_PE_ESL_CLIENT__H_20191228_