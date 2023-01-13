#include "easy.h"
#include "multi.h"
#include "slist.h"

#include <atomic>
#include <boost/format.hpp>

#include "trace.h"

namespace pe { namespace curl {

PE_DECLARE_LOG_CHANNEL(EASYLOG, "curl.easy");
#define THISLOG EASYLOG << "[" << id() << "]: "

Easy::Easy(bool verbose, uint64_t timeout)
  : Easy(curl_easy_init(), verbose, timeout)
{
}

Easy::Easy(handle_t _h, bool _verbose, uint64_t _timeout)
  : handle_(check_(_h))
  , id_(createId())
{
  *errorBuffer_ = 0;

  set(CURLOPT_PRIVATE, this);
  set(CURLOPT_ERRORBUFFER, errorBuffer_);

  set(CURLOPT_READFUNCTION, read_);
  set(CURLOPT_READDATA, this);

  set(CURLOPT_WRITEFUNCTION, write_);
  set(CURLOPT_WRITEDATA, this);

  set(CURLOPT_DEBUGFUNCTION, debug_);
  set(CURLOPT_DEBUGDATA, this);

  set(CURLOPT_XFERINFOFUNCTION, progress_);
  set(CURLOPT_XFERINFODATA, this);

  verbose(_verbose);
  xprogress(true);

  timeout(_timeout);

  //abort if slower than 30 bytes/sec during 60 seconds
  set(CURLOPT_LOW_SPEED_TIME, 60L);
  set(CURLOPT_LOW_SPEED_LIMIT, 30L);
}

Easy::~Easy()
{
  destroy();
}

void Easy::destroy()
{
  if (handle())
  {
#ifdef _CURL_DEBUG
    PE_TRACE(THISLOG << "destroing...");
#endif
    curl_easy_cleanup(handle());
    handle_ = nullptr;
  }
  else
  {
    PE_ERROR(THISLOG << "destroing NULL handle");
  }
}

size_t Easy::createId()
{
  static std::atomic_size_t sid{10000000};
  return sid++;
}

void Easy::check_(CURLcode err, const char* from)
{
  if (err!=CURLE_OK)
  {
    boost::format fmt("%1%: '%2%'");
    string text = (fmt % from % curl_easy_strerror(err)).str();
    throw exceptions::runtime_error(text);
  }
}

Easy::handle_t Easy::check_(handle_t h)
{
  if (!h)
    throw std::invalid_argument("invalid EASY handle");

  return h;
}

void Easy::set(CURLoption option, const StringList& commands) const
{
  set(option, commands.get());
}

void Easy::timeout(uint64_t timeout) const
{
  PE_TRACE(THISLOG << "setting request timeout to " << timeout << " milliseconds...");

  set(CURLOPT_TIMEOUT_MS, static_cast<long>(timeout));
}

void Easy::xprogress(bool enable) const
{
  set(CURLOPT_NOPROGRESS, !enable);
}

void Easy::verbose(bool enable)
{
  set(CURLOPT_VERBOSE, enable);
  verbose_ = enable;
}

void Easy::attach(Multi& multi)
{
#ifdef _CURL_DEBUG
  PE_TRACE(THISLOG << "attach");
#endif
  return multi.attach(*this);
}

void Easy::detach(Multi& multi)
{
#ifdef _CURL_DEBUG
  PE_TRACE(THISLOG << "detach");
#endif
  return multi.detach(*this);
}

void Easy::debug(curl_infotype type, const char* data, size_t size)
{
  PE_UNUSED(type);
  PE_UNUSED(data);
  PE_UNUSED(size);
}

int Easy::progress(curl_off_t downloadTotal,
                   curl_off_t downloaded,
                   curl_off_t uploadTotal,
                   curl_off_t uploaded)
{
  PE_UNUSED(downloadTotal);
  PE_UNUSED(downloaded);
  PE_UNUSED(uploadTotal);
  PE_UNUSED(uploaded);

  return 0;
}

size_t Easy::read(char* buf, size_t size)
{
  PE_UNUSED(buf);
  PE_UNUSED(size);

  //returning 0 will signal end-of-file to the library and cause it to stop the current transfer.
  return 0; //EOF
}

size_t Easy::write(const char* buf, size_t size)
{
  PE_UNUSED(buf);
  PE_UNUSED(size);

  return size;
}

int Easy::debug_(CURL* handle,
                 curl_infotype type,
                 char* data,
                 size_t size,
                 void* userdata)
{
  PE_UNUSED(handle);

  auto self = reinterpret_cast<Easy*>(userdata);
  if (self)
  {
    PE_ASSERT(self->handle()==handle);
    self->debug(type, data, size);
  }

  return 0;
}

int Easy::progress_(void* userdata,
                    curl_off_t dltotal,
                    curl_off_t dlnow,
                    curl_off_t ultotal,
                    curl_off_t ulnow)
{
  auto self = reinterpret_cast<Easy*>(userdata);
  if (!self || self->canceled())
    return -1;

  return self->progress(dltotal, dlnow, ultotal, ulnow);
}

size_t Easy::read_(char* buf, size_t size, size_t nitems, void* userdata)
{
  auto self = reinterpret_cast<Easy*>(userdata);
  if (!self || self->canceled())
    return CURL_READFUNC_ABORT;

  return self->read(buf, size * nitems);
}

size_t Easy::write_(char* buf, size_t size, size_t nmemb, void* userdata)
{
  auto self = reinterpret_cast<Easy*>(userdata);
  if (!self)
    return 0;

  return self->write(buf, size * nmemb);
}

void Easy::cancel()
{
  if (!canceled_.exchange(true))
    canceling();
}

} } //namespace pe { namespace curl {