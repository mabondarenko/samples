#ifndef _PE_API_REDIS_SORTEDSET__H_20190206_
#define _PE_API_REDIS_SORTEDSET__H_20190206_

#include "baseset.h"

namespace pe { namespace redis {

class SortedSet : public BaseSet
{
  using _Base = BaseSet;

public:
  SortedSet(const string& name,
            const Pool& pool,
            unsigned retries=Defaults::kMAX_RETRIES,
            bool trace=false)
    : _Base(name, pool, retries, trace) {}

public:
  using score_t = uint64_t;
  using zmember_t = std::pair<score_t, string>;
  using zmember_set_t = std::set<zmember_t>;

public:
  size_t add(const score_t& score,
             const string& field,
             const string_set_t& options=string_set_t()) const
  {
    return add(zmember_set_t{ {score, field} }, options);
  }

  size_t add(const score_t& score,
             const string_set_t& fields,
             const string_set_t& options=string_set_t()) const
  {
    if (fields.empty())
      return 0;

    auto c = make("ZADD");
    for (const auto& o: options)
      c << o;
    for (const auto& f: fields)
      c << score << f;

    return getInt<size_t>(exec(c));
  }

  size_t add(const zmember_set_t& members, const string_set_t& options=string_set_t()) const
  {
    if (members.empty())
      return 0;

    auto c = make("ZADD");
    for (const auto& o: options)
      c << o;
    for (const auto& m: members)
      c << m.first << m.second;

    return getInt<size_t>(exec(c));
  }

  string_list_t range(const string& zmax=string("+inf"),
                      size_t limit=1000,
                      size_t offset=0,
                      const string& zmin=string("-inf")) const
  {
    auto c = make("ZRANGEBYSCORE");
    c << zmin << zmax;

    if (limit || offset)
    {
      static const string LIMIT("LIMIT");
      c << LIMIT << offset << limit;
    }

    string_list_t result;

    auto r = exec(c);
    if (isArray(r))
    {
      for(const auto& e: r.elements())
      {
        if (isString(e) && !e.str().empty())
          result.push_back(e.str());
      }
    }

    return result;
  }

  static string score(const score_t& value, bool exclusive=true)
  {
    string s = std::to_string(value);
    if (exclusive)
      s.insert(0, 1, '(');
    return s;
  }

  size_t erase(const string& zmin, const string& zmax=string("+inf"))
  {
    return getInt<size_t>(exec(make("ZREMRANGEBYSCORE") << zmin << zmax));
  }

  size_t erase(const score_t& zmin, bool zminexclusive,
               const score_t& zmax, bool zmaxexclusive)
  {
    return erase(score(zmin, zminexclusive), score(zmax, zmaxexclusive));
  }

  size_t trimLeft(const score_t& zmax, bool exclusive=true)
  {
    static const string inf("-inf");
    return erase(inf, score(zmax, exclusive));
  }

  size_t trimRight(const score_t& zmin, bool exclusive=true)
  {
    static const string inf("+inf");
    return erase(score(zmin, exclusive), inf);
  }

public:
  virtual size_t erase(const string_set_t& fields) const override { return _Base::erase("ZREM", fields); }

  virtual string_list_t scan(Cursor& cursor, const string& pattern, unsigned limit=10000) const override
  {
    return _Base::scan("ZSCAN", cursor, pattern, limit);
  }

  virtual size_t size() const override { return _Base::size("ZCARD"); }
};

} } //namespace pe { namespace redis {

#endif //_PE_API_REDIS_SORTEDSET__H_20190206_
