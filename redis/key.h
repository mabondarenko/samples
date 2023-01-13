#ifndef _PE_API_REDIS_KEY__H_20191023_
#define _PE_API_REDIS_KEY__H_20191023_

#include "entity.h"

#include <ostream>

namespace pe { namespace redis {

class Key : public Entity
{
  using _Base = Entity;

public:
  Key(const string& name, const _Base& r)
    : _Base(name, r) {}
  Key(const string& name,
      const Pool& pool,
      unsigned retries=Defaults::kMAX_RETRIES,
      bool trace=false)
    : _Base(name, pool, retries, trace) {}

public:
  Reply get() const { return _Base::get(key()); }
  string getString(const string& defValue=string()) const { return _Base::getString(key(), defValue); }

  template <typename IntType>
  IntType getInt(const IntType& defValue=0) const { return _Base::getInt(key(), defValue); }

  template <typename Type>
  bool set(const Type& value) const { return _Base::set(key(), value); }

  template <typename Type>
  bool set(const Type& value, int64_t expire) const { return _Base::set(key(), value, expire); }

  template <typename Type>
  bool setnx(const Type& value) const { return _Base::setnx(key(), value); }

  template <typename Type>
  bool setnx(const Type& value, int64_t expire) const { return _Base::setnx(key(), value, expire); }
};

} } //namespace pe { namespace redis {

#endif //_PE_API_REDIS_KEY__H_20191023_
