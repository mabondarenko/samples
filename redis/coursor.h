#ifndef _PE_API_REDIS_COURSOR__H_20190206_
#define _PE_API_REDIS_COURSOR__H_20190206_

#include "typedef.h"

namespace pe { namespace redis {

class Cursor
{
public:
  Cursor(const string& value=invalid()) : value_(value) {}

public:
  operator const string&() const { return value_; }
  Cursor& operator=(const string& value) { value_ = value; return *this; }
  operator bool() const { return valid(); }

  bool valid() const { return value_!=invalid(); }
  void reset() { invalidate(); }

protected:
  void invalidate() { value_ = invalid(); }

private:
  static const string& invalid() { static const string s("0"); return s; }

private:
  string value_;
};

} } //namespace pe { namespace redis {

#endif //_PE_API_REDIS_COURSOR__H_20190206_
