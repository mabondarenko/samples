#ifndef _PE_API_REDIS_TYPEDEF__H_20190206_
#define _PE_API_REDIS_TYPEDEF__H_20190206_

#include "types.h"

#include <redis3m/redis3m.hpp>

namespace pe { namespace redis {

//forwards:
class Redis;
class Cursor;
class Pipeline;

using Pool = redis3m::simple_pool::ptr_t;
using Reply = redis3m::reply;
using ReplySet = std::vector<Reply>;
using Command = redis3m::command;
using Transaction = std::vector<Command>;

struct Defaults
{
  enum { kMAX_RETRIES = 50 };
};

} } //namespace pe { namespace redis {

#endif //_PE_API_REDIS_TYPEDEF__H_20190206_
