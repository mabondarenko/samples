#ifndef _PE_CURL_REMOTEDIR__H_20191125_
#define _PE_CURL_REMOTEDIR__H_20191125_

#include "transfer.h"

namespace pe { namespace curl {

class DirectoryEntry
{
public:
  DirectoryEntry(const string& from) { parse(from); }

public:
  const string& name() const { return name_; }
  const string& link() const { return link_; }
  const string& owner() const { return owner_; }
  const string& group() const { return group_; }
  const string& permissions() const { return permissions_; }
  size_t size() const { return size_; }
  bool isDirectory() const { return df_=='d'; }
  bool isSymbolicLink() const { return df_=='l'; }
  bool isRegularFile() const { return df_=='-'; }

private:
  void parse(const string& from) noexcept(false);

private:
  string name_;
  string link_;
  string owner_;
  string group_;
  string permissions_;
  size_t size_ = 0;
  char df_ = 0;
};

class RemoteDirectory : public Transfer
{
  using _Base = Transfer;
  using Entries = std::vector<DirectoryEntry>;

public:
  RemoteDirectory(const ServicePtr& service,
                  const string& url=string(),
                  bool verbose=false,
                  uint64_t timeout=REQUEST_TIMEOUT_MS);

public:
  const Entries& content() const { return entries_; }
  const string& data() const { return buffer_; }

protected:
  virtual void done(int error, const char* text) override;
  virtual size_t write(const char* buf, size_t size) override;

private:
  void parse(const string& from) noexcept(false);

private:
  string buffer_;
  Entries entries_;
};

} } //namespace pe { namespace curl {

#endif //#ifndef _PE_CURL_REMOTEDIR__H_20191125_