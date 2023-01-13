#include "syncwait.h"

#include <boost/chrono.hpp>

#include "transfer.h"

#include "trace.h"

namespace pe { namespace curl {

PE_DECLARE_LOG_CHANNEL(SYNCLOG, "curl.synchro");
#define THISLOG SYNCLOG << "[" << id() << "]: "

using guard_t = boost::unique_lock<boost::mutex>;

Synchro::Synchro(const TransferPtr& transfer)
  : transfer_(transfer)
  , id_(transfer->id())
{
}

Synchro::~Synchro()
{
#ifdef _CURL_DEBUG
  PE_DEBUG(THISLOG << "destroying...");
#endif
}

void Synchro::finish()
{
#ifdef _CURL_DEBUG
  PE_TRACE(THISLOG << "finish()");
#endif

  {
    guard_t guard(mutex_);
    finished_ = true;
  }

  condition_.notify_all();

  PE_TRACE(THISLOG << "finished");
}

bool Synchro::wait(int msec)
{
  guard_t guard(mutex_);

  if (finished_)
    return true;

  auto abst = boost::chrono::steady_clock::now() + boost::chrono::milliseconds(msec);

  if (condition_.wait_until(guard, abst)==boost::cv_status::no_timeout)
  {
#ifdef _CURL_DEBUG
    PE_TRACE(THISLOG << "wait done!");
#endif
    return true;
  }

  if (finished_)
  {
    PE_ERROR(THISLOG << "boost condition wait bug!");
    return true;
  }

  PE_DEBUG(THISLOG << "wait timeout!");

  if (auto t = transfer())
    t->cancel();

  return false;
}

} } //namespace pe { namespace curl {
