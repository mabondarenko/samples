#ifndef _PE_CURL_ENGINE__H_20200307_
#define _PE_CURL_ENGINE__H_20200307_

#include "types.h"

#include <boost/asio/io_service.hpp>
#include <boost/thread/thread.hpp>
#include <boost/scoped_ptr.hpp>

namespace pe { namespace curl {

class Service;

class Engine : noncopyable
{
  using ServicePtr = std::shared_ptr<Service>;

public:
  Engine(const string& name);
  virtual ~Engine(void);

public:
  const string& name(void) const;
  boost::asio::io_service& ios(void) { return ios_; }
  const ServicePtr& service() const { return service_; }

private:
  void work(void);
  void test(void);

private:
  boost::asio::io_service ios_;
  ServicePtr service_;
  boost::thread worker_;
  std::unique_ptr<boost::asio::io_service::work> work_; //must be after worker_ !!!
};

} } //namespace pe { namespace curl {

#endif //_PE_CURL_ENGINE__H_20200307_