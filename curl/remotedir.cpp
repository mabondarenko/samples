#include "remotedir.h"

#include <regex>

#include "tools/str.h"

namespace pe { namespace curl {

RemoteDirectory::RemoteDirectory(const ServicePtr& service,
                                 const string& url,
                                 bool verbose,
                                 uint64_t timeout)
  : _Base(service, url, verbose, timeout)
{
}

void RemoteDirectory::done(int error, const char* text)
{
  try
  {
    if (!error)
      parse(buffer_);
  }
  catch(const std::exception& e)
  {
    _Base::done(CURLE_FTP_BAD_FILE_LIST, e.what());
    return;
  }

  _Base::done(error, text);
}

size_t RemoteDirectory::write(const char* buf, size_t size)
{
  buffer_.append(buf, size);
  return size;
}

void RemoteDirectory::parse(const string& from)
{
  string_list_t entries;
  tools::str::split(entries, from, [](char c) { return c=='\n'; });

  for(const auto& e: entries)
    entries_.emplace_back(DirectoryEntry(e));
}

void DirectoryEntry::parse(const string& from)
{
  static const std::regex expression(
    "^([\\-dbclps])"                // Directory flag [1]
    "([\\-rwxs]{9})\\s+"            // Permissions [2]
    "(\\d+)\\s+"                    // Number of items [3]
    "(\\w+)\\s+"                    // File owner [4]
    "(\\w+)\\s+"                    // File group [5]
    "(\\d+)\\s+"                    // File size in bytes [6]
    "(\\w{3}\\s+\\d{1,2}\\s+"       // 3-char month and 1/2-char day of the month [7]
    "(?:\\d{1,2}:\\d{1,2}|\\d{4}))" // Time or year (need to check conditions) [+= 7]
    "\\s+(.+)$"                     // File/directory name [8]
  );

  std::smatch m;
  if (std::regex_match(from, m, expression) && m.size()==9)
  {
    string df = m[1];
    df_ = df[0];
    permissions_ = m[2];
    owner_ = m[4];
    group_ = m[5];
    size_ = std::stoull(m[6]);

    if (!isSymbolicLink())
      name_ = m[8];
    else
    {
      static const std::regex linkexp("^(.+) -> (.+)$");

      std::smatch lm;
      string fname = m[8];
      if (std::regex_match(fname, lm, linkexp) && lm.size()==3)
      {
        name_ = lm[1];
        link_ = lm[2];
      }
      else
      {
        std::runtime_error("bad file name in directory entry string");
      }
    }
  }
  else
  {
    std::runtime_error("bad remote directory entry string");
  }
}

} } //namespace pe { namespace curl {