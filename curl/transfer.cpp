#include "transfer.h"

#include "service.h"
#include "trace.h"
#include "url.h"

namespace pe { namespace curl {

PE_DECLARE_LOG_CHANNEL(TRFLOG, "curl.transfer");
#define THISLOG TRFLOG << "[" << id() << "]: "

Transfer::Transfer(const ServicePtr& _service,
                   const string& _url,
                   bool _verbose,
                   uint64_t timeout)
  : _Base(_verbose, timeout)
  , service_(_service)
{
  if (!_url.empty())
    url(_url);

  PE_DEBUG(THISLOG << "created...");
}

Transfer::~Transfer()
{
  PE_DEBUG(THISLOG << "destroying...");

  detach();
  //destroy();
}

void Transfer::exec(ResultHandler&& h)
{
  done_ = h;
  try
  {
    attach();
  }
  catch(const std::exception&)
  {
    reset();
    throw;
  }
}

void Transfer::reset()
{
  done_ = ResultHandler();
  read_ = ReadHandler();
  write_ = WriteHandler();
}

void Transfer::dispatch(const CURLMsg* msg)
{
#ifdef _CURL_DEBUG
  PE_TRACE(THISLOG << "dispatch(): " << msg->msg);
#endif

  if (msg->msg==CURLMSG_DONE)
  {
    auto result = msg->data.result;

#if(1)
    switch(result)
    {
    case CURLE_OK:
      PE_DEBUG(THISLOG << "'" << url() << "' successfully done.");
      break;
    case CURLE_ABORTED_BY_CALLBACK:
      PE_DEBUG(THISLOG << "'" << url() << "' canceled.");
      break;
    default:
      PE_ERROR(THISLOG << "'" << url() << "' failed with error " << result << " (" << curl_easy_strerror(result) << ").");
      break;
    }

    auto self = shared_from_this();
    service_->ios().post([this, self, result]() {
      done(result, curl_easy_strerror(result));
    });
#else
  done(result, curl_easy_strerror(result));
#endif
  }
}

void Transfer::done(int error, const char* text)
{
  auto self = shared_from_this();
  if (done_)
  {
    try
    {
      done_(self, error, text);
    }
    catch(const std::exception& e)
    {
      PE_ERROR(THISLOG << "exception in user done handler: " << e.what());
    }
  }

  reset();
}

void Transfer::attach()
{
  if (!attached_)
  {
    _Base::attach(*service_.get());
    attached_ = true;
  }
}

void Transfer::detach()
{
  if (attached_)
  {
    _Base::detach(*service_.get());
    attached_ = false;
  }
}

void Transfer::debug(curl_infotype type, const char* data, size_t size)
{
  string text(data, size);

  if (type==CURLINFO_TEXT ||
      type==CURLINFO_HEADER_IN ||
      type==CURLINFO_HEADER_OUT)
  {
    auto end = data + size;
    while(--end>=data)
    {
      if (*end=='\n' || *end=='\r')
        --size;
      else
        break;
    }

    text.resize(size);
  }

  switch(type)
  {
  case CURLINFO_TEXT:
    PE_DEBUG(THISLOG << "* " << text);
    break;
  case CURLINFO_HEADER_IN:
    PE_DEBUG(THISLOG << "< " << text);
    break;
  case CURLINFO_HEADER_OUT:
    PE_DEBUG(THISLOG << "> " << text);
    break;
  case CURLINFO_DATA_IN:
    //PE_DEBUG(THISLOG << "<<< '\n" << text << "'.");
    break;
  case CURLINFO_DATA_OUT:
    //PE_DEBUG(THISLOG << ">>> '\n" << text << "'.");
    break;
  case CURLINFO_SSL_DATA_IN:
    //PE_DEBUG(THISLOG << "SSL <<< '\n" << text << "'.");
    break;
  case CURLINFO_SSL_DATA_OUT:
    //PE_DEBUG(THISLOG << "SSL >>> '\n" << text << "'.");
    break;
  default:
    break;
  }
}

int Transfer::progress(curl_off_t downloadTotal,
                       curl_off_t downloaded,
                       curl_off_t uploadTotal,
                       curl_off_t uploaded)
{
  return _Base::progress(downloadTotal,
                         downloaded,
                         uploadTotal,
                         uploaded);
}

size_t Transfer::read(char* buf, size_t size)
{
  if (read_)
  {
    try
    {
      return read_(shared_from_this(), buf, size);
    }
    catch(const std::exception& e)
    {
      PE_ERROR(THISLOG << "execption in user read handler: " << e.what());
    }
  }

  return 0;
}

size_t Transfer::write(const char* buf, size_t size)
{
  if (write_)
  {
    try
    {
      return write_(shared_from_this(), buf, size);
    }
    catch(const std::exception& e)
    {
      PE_ERROR(THISLOG << "execption in user write handler: " << e.what());
    }
  }

  return size;
}

void Transfer::canceling()
{
  PE_DEBUG(THISLOG << "canceling...");
}

string Transfer::url() const
{
  const char* ct = nullptr;
  try
  {
    ct = get<const char*>(CURLINFO_EFFECTIVE_URL);
  }
  catch(const std::exception& e)
  {
    PE_ERROR(THISLOG << "failed to get effective URL: " << e.what());
  }

  return ct ? string(ct) : string();
}

Url Transfer::getUrl() const
{
  return Url(url());
}

} } //namespace pe { namespace curl {
