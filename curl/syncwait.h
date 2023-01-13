#ifndef _PE_API_CURL_SYNCWAIT__H_20200307_
#define _PE_API_CURL_SYNCWAIT__H_20200307_

#include "types.h"

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

namespace pe { namespace curl {

class Transfer;

class Synchro : noncopyable
{
  using TransferPtr = std::shared_ptr<Transfer>;
  using TransferRef = std::weak_ptr<Transfer>;

public:
  Synchro(const TransferPtr& transfer);
  ~Synchro();

public:
  size_t id() const { return id_; }
  void finish();
  bool wait(int timeoutms);
  TransferPtr transfer() const { return transfer_.lock(); }

private:
  TransferRef transfer_;
  size_t id_;
  boost::mutex mutex_;
  boost::condition condition_;
  bool finished_ = false;
};

} } //namespace pe { namespace curl {

#endif //#ifndef _PE_API_CURL_SYNCWAIT__H_20200307_
