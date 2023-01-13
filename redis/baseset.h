#ifndef _PE_API_REDIS_BASESET__H_20190206_
#define _PE_API_REDIS_BASESET__H_20190206_

#include "container.h"

namespace pe { namespace redis {

class BaseSet : public Container
{
  using _Base = Container;

public:
  BaseSet(const string& name,
          const Pool& pool,
          unsigned retries=Defaults::kMAX_RETRIES,
          bool trace=false)
    : _Base(name, pool, retries, trace) {}

public:
  virtual size_t erase(const string& field) const override { return erase(string_set_t{field}); }
  virtual size_t erase(const string_set_t& fields) const=0;
  virtual string_list_t scan(Cursor& cursor, const string& pattern, unsigned limit=10000) const=0;

protected:
  size_t erase(const char* cmd, const string_set_t& fields) const
  {
    if (fields.empty())
      return 0;

    auto c = make(cmd);
    for (const auto& f: fields)
      c << f;

    return getInt<size_t>(exec(c));
  }

  string_list_t scan(const char* cmd, Cursor& cursor, const string& pattern, unsigned limit=10000) const
  {
    auto c = make(cmd);
    return _Base::scan(c, cursor, pattern, limit);
  }
};

} } //namespace pe { namespace redis {

#endif //_PE_API_REDIS_BASESET__H_20190206_
