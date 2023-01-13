#ifndef _PE_API_REDIS_POOL__H_20190702_
#define _PE_API_REDIS_POOL__H_20190702_

#include "typedef.h"

namespace pe { namespace redis {

Pool create_simple_pool(const string& addr);

} } //namespace pe { namespace redis {

#endif //_PE_API_REDIS_POOL__H_20190702_
