#ifndef _PE_API_REDIS_SCRIPT__H_20191021_
#define _PE_API_REDIS_SCRIPT__H_20191021_

#include "redis.h"

#include <ostream>

namespace pe { namespace redis {

class Script : protected Redis
{
  using _Base = Redis;

public:
  Script(const _Base& redis,
         const string& sha1=string());
  Script(const Pool& pool,
         const string& sha1=string(),
         unsigned retries=Defaults::kMAX_RETRIES,
         bool trace=false);

public:
  static Script load(const Redis& redis, const string& script)
  {
    return Script(script, redis);
  }

  static Script load(const Redis& redis, const string_list_t& script)
  {
    return load(redis, build(script));
  }

  static Script load(const Pool& pool,
                     const string& script,
                     unsigned retries=Defaults::kMAX_RETRIES,
                     bool trace=false)
  {
    return Script(script, pool, retries, trace);
  }

  static Script load(const Pool& pool,
                     const string_list_t& script,
                     unsigned retries=Defaults::kMAX_RETRIES,
                     bool trace=false)
  {
    return load(pool, build(script), retries, trace);
  }

public:
  static string build(const string_list_t& script);

public:
  const string& compile(const string& script);
  const string& compile(const string_list_t& script) { return compile(build(script)); }
  bool exists() const;
  const string& key() const { return sha1(); }
  const string& sha1() const { return sha1_; }
  operator const string&() const { return key(); }
  operator bool() const { return exists(); }

public:
  template<typename... Args>
  Reply run(uint32_t keys, const Args&... args)
  {
    return exec(evalsha(sha1(), keys, args...));
  }

  template<typename... Args>
  Reply run(const string& script, uint32_t keys, const Args&... args)
  {
    return exec(eval(script, keys, args...));
  }

  template<typename... Args>
  static Command evalsha(const string& sha1, uint32_t keys, const Args&... args)
  {
    static const string EVAL("EVALSHA");
    return prepare(Command(EVAL) << sha1 << keys, args...);
  }

  template<typename... Args>
  static Command eval(const string& script, uint32_t keys, const Args&... args)
  {
    static const string EVAL("EVAL");
    return prepare(Command(EVAL) << script << keys, args...);
  }

protected:
  template<typename... Args>
  static Command& prepare(Command& cmd, const Args&... args)
  {
    int dummy[sizeof...(Args)] = { (cmd << args, 0)... };
    PE_UNUSED(dummy);
    return cmd;
  }

protected:
  Script(const string& script, const Redis& redis);
  Script(const string& script,
         const Pool& pool,
         unsigned retries=Defaults::kMAX_RETRIES,
         bool trace=false);
  string load(const string& script) const;

private:
  static Command SCRIPT();

private:
  string sha1_;
};

inline std::ostream& operator<<(std::ostream& os, const Script& s)
{
  return os << s.key();
}

} } //namespace pe { namespace redis {

#endif //_PE_API_REDIS_ENTITY__H_20190206_
