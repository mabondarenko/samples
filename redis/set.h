#ifndef _PE_API_REDIS_SET__H_20190206_
#define _PE_API_REDIS_SET__H_20190206_

#include "baseset.h"

namespace pe { namespace redis {

class Set : public BaseSet
{
  using _Base = BaseSet;

public:
  Set(const string& name,
      const Pool& pool,
      unsigned retries=Defaults::kMAX_RETRIES,
      bool trace=false)
    : _Base(name, pool, retries, trace) {}

public:
  size_t add(const string& field) const { return add(string_set_t{field}); }

  size_t add(const string_set_t& fields) const
  {
    if (fields.empty())
      return 0;

    auto c = make("SADD");
    for (const auto& f: fields)
      c << f;

    return getInt<size_t>(exec(c));
  }

  string_set_t pop(size_t count=1) const
  {
    string_set_t result;

    if (count)
    {
      auto r = exec(make("SPOP") << count);
      if (isArray(r))
      {
        for(const auto& e: r.elements())
        {
          if (isString(e) && !e.str().empty())
            result.insert(e.str());
        }
      }
    }

    return result;
  }

public:
  virtual size_t erase(const string_set_t& fields) const override { return _Base::erase("SREM", fields); }
  virtual string_list_t scan(Cursor& cursor, const string& pattern, unsigned limit=10000) const override
  {
    return _Base::scan("SSCAN", cursor, pattern, limit);
  }

  virtual size_t size() const override { return _Base::size("SCARD"); }
};

} } //namespace pe { namespace redis {

#endif //_PE_API_REDIS_SET__H_20190206_
