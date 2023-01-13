#include "script.h"

#include <sstream>

namespace pe { namespace redis {

Script::Script(const _Base& redis, const string& sha1)
  : _Base(redis)
  , sha1_(sha1)
{
}

Script::Script(const Pool& pool,
               const string& sha1,
               unsigned retries,
               bool trace)
  : _Base(pool, retries, trace)
  , sha1_(sha1)
{
}

Script::Script(const string& script, const Redis& redis)
  : _Base(redis)
  , sha1_(load(script))
{
}

Script::Script(const string& script,
               const Pool& pool,
               unsigned retries,
               bool trace)
  : _Base(pool, retries, trace)
  , sha1_(load(script))
{
}

string Script::build(const string_list_t& script)
{
  std::stringstream ss;
  for(const auto& line: script)
    ss << line << std::endl;
  return ss.str();
}

const string& Script::compile(const string& script)
{
  sha1_ = load(script);
  return sha1();
}

bool Script::exists() const
{
  if (sha1().empty())
    return false;

  static string EXISTS("EXISTS");

  try
  {
    return getInt<int>(exec(SCRIPT() << EXISTS << sha1()))==1;
  }
  catch(...)
  {
  }
  return false;
}

string Script::load(const string& script) const
{
  static const string LOAD("LOAD");
  return getString(exec(SCRIPT() << LOAD << script));
}

Command Script::SCRIPT()
{
  static const string cmd("SCRIPT");
  return Command(cmd);
}

} } //namespace pe { namespace redis {
