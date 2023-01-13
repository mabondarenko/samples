#ifndef _PE_ESL_COMMAND__H_20200602_
#define _PE_ESL_COMMAND__H_20200602_

#include "types.h"

#include <sstream>

namespace pe { namespace esl {

class Command
{
public:
  Command() {}
  explicit Command(const string& s) : Command(s.c_str()) {}
  explicit Command(const char* s) { put(s); }

  template<typename T>
  Command& operator<<(const T& val) { return put(val); }

  string str() const { return s_.str(); }

private:
  template<typename T>
  Command& put(const T& val)
  {
    if (++args>1)
      s_ << " ";

    s_ << val;
    return *this;
  }

private:
  std::stringstream s_;
  size_t args = 0;
};

} } //namespace pe { namespace esl {

#endif //#ifndef _PE_ESL_COMMAND__H_20200602_