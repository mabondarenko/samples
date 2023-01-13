#ifndef _PE_API_REDIS_ENTITY__H_20190206_
#define _PE_API_REDIS_ENTITY__H_20190206_

#include "redis.h"

#include <ostream>

namespace pe { namespace redis {

class Entity : public Redis
{
  using _Base = Redis;

public:
  Entity(const string& name, const _Base& r)
    : _Base(r)
    , name_(name)
  {
  }
  Entity(const string& name,
         const Pool& pool,
         unsigned retries=Defaults::kMAX_RETRIES,
         bool trace=false)
    : _Base(pool, retries, trace)
    , name_(name)
  {
  }

public:
  bool exists() const { return _Base::exists(key()); }
  bool expire(int64_t value) const { return _Base::expire(key(), value); }
  const string& key() const { return name(); }
  Command make(const char* cmd) const { return Command(cmd) << key(); }
  Command make(const string& cmd) const { return Command(cmd) << key(); }
  const string& name() const { return name_; }
  bool remove() const { return _Base::remove(key()); }
  int64_t ttl() const { return _Base::ttl(key()); }

public:
  operator const string&() const { return key(); }
  operator bool() const { return exists(); }

private:
  string name_;
};

inline std::ostream& operator<<(std::ostream& os, const Entity& e)
{
  return os << e.key();
}

} } //namespace pe { namespace redis {

#endif //_PE_API_REDIS_ENTITY__H_20190206_
