#ifndef _PE_API_REDIS_CONTAINER__H_20190206_
#define _PE_API_REDIS_CONTAINER__H_20190206_

#include "entity.h"

namespace pe { namespace redis {

class Container : public Entity
{
  using _Base = Entity;

public:
  Container(const string& name,
            const Pool& pool,
            unsigned retries=Defaults::kMAX_RETRIES,
            bool trace=false)
    : _Base(name, pool, retries, trace) {}

public:
  virtual size_t erase(const string& field) const=0;
  virtual size_t size() const=0;

protected:
  size_t size(const char* cmd) const { return getInt<size_t>(exec(make(cmd))); }
};

} } //namespace pe { namespace redis {

#endif //_PE_API_REDIS_CONTAINER__H_20190206_
