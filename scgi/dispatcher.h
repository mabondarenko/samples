#ifndef _PE_SCGI_DISPATCHER__20190228_
#define _PE_SCGI_DISPATCHER__20190228_

#include "../types.h"

#include <functional>

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>

#include "mutex.h"

namespace pe { namespace scgi {

class Connection;
class Request;

class Dispatcher : public std::enable_shared_from_this<Dispatcher>
                 , noncopyable
{
  using ConnectionPtr = std::shared_ptr<Connection>;
  using RequestPtr = std::shared_ptr<Request>;

public:
  using RequestHandler = std::function<void(const RequestPtr&)>;

public:
  Dispatcher(boost::asio::io_service& ios);
  virtual ~Dispatcher();

public:
  void add(const string& pattern, const RequestHandler& handler);
  void remove(const string& pattern);
  void cancel();

public:
  boost::asio::io_service& ios() { return ios_; }

public:
  bool onMessage(const ConnectionPtr& connection, const buffer& msg);

protected:
  void process(const RequestPtr& request);
  void exec(const RequestPtr& request);
  RequestHandler find(const string& uri) const;

private:
  static string createPattern(const string& from);

private:
  boost::asio::io_service& ios_;
  boost::asio::io_service::strand strand_;
  std::map<string, RequestHandler> handlers_;
  mutex_t mtx_;
};

} } //namespace pe { namespace scgi {

#endif //#ifndef _PE_SCGI_DISPATCHER__20190228_
