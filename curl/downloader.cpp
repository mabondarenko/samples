#include "downloader.h"

#include <boost/filesystem.hpp>

#include "trace.h"

namespace pe { namespace curl {

PE_DECLARE_LOG_CHANNEL(TRFLOG, "curl.downloader");
#define THISLOG TRFLOG << "[" << id() << "]: "

Downloader::Downloader(const ServicePtr& service,
                       const string& url,
                       const string& file,
                       uint64_t timeout)
  : _Base(service, url, timeout)
  , file_(file)
{
}

void Downloader::done(int error, const char* text)
{
  try
  {
    close(error);
  }
  catch(const std::exception& e)
  {
    _Base::done(CURLE_WRITE_ERROR, e.what());
    return;
  }

  _Base::done(error, text);
}

size_t Downloader::write(const char* buf, size_t size)
{
  if (!open())
  {
    PE_ERROR(THISLOG << "failed to open temporary file");
    return size-1;
  }

  if (!out_.write(buf, size))
  {
    PE_ERROR(THISLOG << "failed to write temporary file");
    return size-1;
  }

  return size;
}

bool Downloader::open()
{
  if (out_.is_open())
    return true;

  static bool inited = false;
  static auto model = boost::filesystem::temp_directory_path();
  if (!inited)
  {
    model /= "pe-downloader-%%%%-%%%%-%%%%-%%%%.tmp";
    inited = true;
  }

  for(size_t i=0; i<10; ++i)
  {
    boost::system::error_code ec;
    auto p = boost::filesystem::unique_path(model, ec);
    if (!ec)
    {
      tmpfile_ = p.string();
      out_.open(tmpfile_, std::ofstream::binary);
      if (out_)
        return true;
    }
  }

  return false;
}

void Downloader::close(int error)
{
  if (!out_.is_open())
    return;

  out_.close();

  if (!error)
    boost::filesystem::rename(tmpfile_, file_); //throws
  else
  {
    boost::system::error_code ec;
    boost::filesystem::remove(tmpfile_, ec); //no throw
  }
}

} } //namespace pe { namespace curl {