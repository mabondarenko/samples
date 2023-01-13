#ifndef _PE_API_REDIS_HASH__H_20190206_
#define _PE_API_REDIS_HASH__H_20190206_

#include "baseset.h"

namespace pe { namespace redis {

class Hash : public BaseSet
{
  using _Base = BaseSet;

public:
  Hash(const string& name,
       const Pool& pool,
       unsigned retries=Defaults::kMAX_RETRIES,
       bool trace=false)
  : _Base(name, pool, retries, trace) {}

public:
  using member_t = std::pair<string, string>;
  using member_set_t = std::set<member_t>;

public:
  string getValue(const string& field) const { return getString(exec(make("HGET") << field)); }

  size_t set(const member_set_t& members) const
  {
    if (members.empty())
      return 0;

    auto c = make("HSET");
    for (const auto& m: members)
      c << m.first << m.second;

    return getInt<size_t>(exec(c));
  }

  size_t setValue(const string& field, const string& value) const { return set(member_set_t{ {field, value} }); }

public:
  virtual size_t erase(const string_set_t& fields) const override { return _Base::erase("HDEL", fields); }

  virtual string_list_t scan(Cursor& cursor, const string& pattern, unsigned limit=10000) const override
  {
    return _Base::scan("HSCAN", cursor, pattern, limit);
  }

  virtual size_t size() const override { return _Base::size("HLEN"); }
};

} } //namespace pe { namespace redis {

#endif //_PE_API_REDIS_HASH__H_20190206_
