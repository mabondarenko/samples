#ifndef _PE_API_REDIS_REDIS__H_20190129_
#define _PE_API_REDIS_REDIS__H_20190129_

#include "typedef.h"

namespace pe { namespace redis {

class Redis
{
public:
  Redis(const Pool& pool,
        unsigned retries=Defaults::kMAX_RETRIES,
        bool trace=false)
    : pool_(pool)
    , retries_(retries)
    , trace_(trace)
  {
  }

  virtual ~Redis()=default;

public:
  Reply exec(const Command& cmd) const;
  Reply exec(const string_list_t& cmd) const;
  Reply exec(const Transaction& tr) const;

  inline bool exists(const string& key) const;
  inline bool expire(const string& key, int64_t value) const;

  inline Reply get(const string& key) const;
  string_list_t get(const string_list_t& keys) const;

  string getString(const string& key, const string& defValue=string()) const { return getString(get(key), defValue); }
  inline string getString(const Reply& r, const string& defValue=string()) const;

  template <typename IntType>
  IntType getInt(const Reply& r, const IntType& defValue=0) const;
  template <typename IntType>
  IntType getInt(const string& key, const IntType& defValue=0) const { return getInt(get(key), defValue); }

  bool remove(const string& key) const { return remove(string_list_t(1, key))==1; }
  size_t remove(const string_list_t& keys) const;
  size_t removex(const string& pattern, unsigned batch=10000) const;

  inline string_list_t scan(Cursor& cursor, const string& pattern, unsigned limit=10000) const;
  string_list_t scan(Command& command,
                     Cursor& cursor,
                     const string& pattern,
                     unsigned limit=10000) const;

  template <typename Type>
  bool set(const string& key, const Type& value) const;

  template <typename Type>
  bool set(const string& key, const Type& value, int64_t expire) const;

  template <typename Type>
  bool setnx(const string& key, const Type& value) const;

  template <typename Type>
  bool setnx(const string& key, const Type& value, int64_t expire) const;

  inline int64_t ttl(const string& key) const;

public:
  static bool isString(const Reply& r)  { return r.type()==Reply::type_t::STRING; }
  static bool isArray(const Reply& r)   { return r.type()==Reply::type_t::ARRAY; }
  static bool isInteger(const Reply& r) { return r.type()==Reply::type_t::INTEGER; }
  static bool isNULL(const Reply& r)    { return r.type()==Reply::type_t::NIL; }
  static bool isError(const Reply& r)   { return r.type()==Reply::type_t::ERROR; }
  static bool isStatus(const Reply& r)  { return r.type()==Reply::type_t::STATUS; }
  static const string_list_t& args(const Command& cmd)
  {
    return static_cast<const string_list_t&>(const_cast<Command&>(cmd));
  }
  static string debugString(const Command& cmd)
  {
    return const_cast<Command&>(cmd).toDebugString();
  }

public:
  const Pool& pool() const { return pool_; }
  unsigned retries() const { return retries_; }
  bool trace() const { return trace_; }

private:
  Pool pool_;
  unsigned retries_;
  bool trace_;
};

bool Redis::expire(const string& key, int64_t value) const
{
  static const string cmd("EXPIRE");
  return getInt<int>(exec(Command(cmd) << key << value))==1;
}

bool Redis::exists(const string& key) const
{
  static const string cmd("EXISTS");
  return getInt<int>(exec(Command(cmd) << key))==1;
}

Reply Redis::get(const string& key) const
{
  static const string cmd("GET");
  return exec(Command(cmd) << key);
}

string Redis::getString(const Reply& r, const string& defValue) const
{
  if (isString(r))
    return r.str();
  else if (isInteger(r))
    return std::to_string(r.integer());
  return defValue;
}

template <typename IntType>
IntType Redis::getInt(const Reply& r, const IntType& defValue) const
{
  if (isString(r))
    return static_cast<IntType>(std::atoll(r.str().c_str()));
  else if (isInteger(r))
    return static_cast<IntType>(r.integer());

  return defValue;
}

string_list_t Redis::scan(Cursor& cursor, const string& pattern, unsigned limit) const
{
  auto cmd = Command("SCAN");
  return scan(cmd, cursor, pattern, limit);
}

template <typename Type>
bool Redis::set(const string& key, const Type& value) const
{
  static const string cmd("SET");
  return !isError(exec(Command(cmd) << key << value));
}

template <typename Type>
bool Redis::set(const string& key, const Type& value, int64_t expire) const
{
  static const string cmd("SETEX");
  return !isError(exec(Command(cmd) << key << expire << value));
}

template <typename Type>
bool Redis::setnx(const string& key, const Type& value) const
{
  static const string cmd("SETNX");
  return !isError(exec(Command(cmd) << key << value));
}

template <typename Type>
bool Redis::setnx(const string& key, const Type& value, int64_t expire) const
{
  static const string cmd("SET");
  static const string EX("EX");
  static const string NX("NX");
  return !isError(exec(Command(cmd) << key << value << EX << expire << NX));
}

int64_t Redis::ttl(const string& key) const
{
  static const string cmd("TTL");
  return getInt<int64_t>(exec(Command(cmd) << key));
}

} } //namespace pe { namespace redis {

#endif //_PE_API_REDIS_REDIS__H_20190129_
