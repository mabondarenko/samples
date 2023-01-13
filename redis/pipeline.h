#ifndef _PE_API_REDIS_PIPELINE__H_20191019_
#define _PE_API_REDIS_PIPELINE__H_20191019_

#include "redis.h"

namespace pe { namespace redis {

class Pipeline : protected Redis
{
  using _Base = Redis;

public:
  Pipeline(const _Base& r) : _Base(r) {}
  Pipeline(const Pool& pool,
           unsigned retries=Defaults::kMAX_RETRIES,
           bool trace=false)
    : _Base(pool, retries, trace) {}

public:
  virtual Pipeline& append(const Command& cmd);
  virtual Pipeline& append(Command&& cmd);

public:
  Pipeline& operator<<(const Command& cmd) { return append(cmd); }
  Pipeline& operator<<(Command&& cmd) { return append(std::move(cmd)); }

public:
  ReplySet exec() const;
  void reset() { tr_.clear(); }
  size_t size() const { return tr().size(); }
  const Transaction& tr() const { return tr_; }

private:
  Transaction tr_;
};

} } //namespace pe { namespace redis {

#endif //_PE_API_REDIS_PIPELINE__H_20191019_
