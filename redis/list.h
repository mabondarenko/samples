#ifndef _PE_API_REDIS_LIST__H_20190206_
#define _PE_API_REDIS_LIST__H_20190206_

#include "container.h"

namespace pe { namespace redis {

class List : public Container
{
  using _Base = Container;

public:
  List(const string& name,
       const Pool& pool,
       unsigned retries=Defaults::kMAX_RETRIES,
       bool trace=false)
    : _Base(name, pool, retries, trace) {}

public:
  string first(const string& defValue=string()) const
  {
    return getString(exec(make("LPOP")), defValue);
  }

  string last(const string& defValue=string()) const
  {
    return getString(exec(make("RPOP")), defValue);
  }

  size_t push(const string& value) const { return push(string_list_t(1, value)); }
  size_t push_back(const string& value) const { return push_back(string_list_t(1, value)); }
  size_t push(const string_list_t& values) const { return push("LPUSH", values); }
  size_t push_back(const string_list_t& values) const { return push("RPUSH", values); }
  string pop(const List& dst) const //RPOPLPUSH (this - RPOP, dst - LPUSH)
  {
    return getString(exec(make("RPOPLPUSH") << dst.key()));
  }

public:
  const List& operator<<(const string& value) const { push_back(value); return *this; }
  string operator>>(const List& dst) const { return pop(dst); }

public:
  size_t erase(const string& field, int64_t count) const
  {
    return getInt<size_t>(exec(make("LREM") << count << field));
  }
  virtual size_t erase(const string& field) const override { return erase(field, 0); }
  virtual size_t size() const override { return _Base::size("LLEN"); }

private:
  size_t push(const char* cmd, const string_list_t& values) const
  {
    if (values.empty())
      return 0;

    auto c = make(cmd);
    for (const auto& v: values)
      c << v;

    return getInt<size_t>(exec(c));
  }
};

} } //namespace pe { namespace redis {

#endif //_PE_API_REDIS_LIST__H_20190206_
