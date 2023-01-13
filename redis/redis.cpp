#include "redis.h"

#include "coursor.h"

#include "trace.h"

namespace pe { namespace redis {

PE_DECLARE_LOG_CHANNEL(THISLOG, "redis: ");

Reply Redis::exec(const Command& cmd) const
{
  if (trace_)
  {
    PE_TRACE(THISLOG << "exec: " << debugString(cmd));
  }
  return exec(args(cmd));
}

Reply Redis::exec(const string_list_t& cmd) const
{
  try
  {
    return pool_->run_with_connection<redis3m::reply>([&](redis3m::connection::ptr_t conn)
    {
      auto r = conn->run(cmd);

      if (isError(r))
      {
        static const string NO_COMMAND("<NO COMMAND>");
        const auto& cmdname = cmd.empty() ? NO_COMMAND : cmd[0];
        throw exceptions::runtime_error("redis error on command '" + cmdname + "' " + r.str());
      }

      return r;
    },
    retries());
  }
  catch(std::exception& e)
  {
    PE_ERROR(THISLOG << "exception occurried while executing command: '" << e.what() << "'.");
    throw;
  }
}

Reply Redis::exec(const Transaction& tr) const
{
  try
  {
    return pool_->run_with_connection<redis3m::reply>([&](redis3m::connection::ptr_t conn)
    {
      static const string MULTI("MULTI");
      conn->run(Command(MULTI));

      for (const auto& cmd: tr)
      {
        if (trace_)
        {
          PE_TRACE(THISLOG << "run: " << debugString(cmd));
        }

        auto r = conn->run(args(cmd));
        if (isError(r))
        {
          PE_ERROR(THISLOG << "error occuried while executing transaction: " << r.str());

          static const string DISCARD("DISCARD");
          conn->run(Command(DISCARD));
          return r;
        }
      }

      static const string EXEC("EXEC");
      return conn->run(Command(EXEC));
    },
    retries());
  }
  catch(std::exception& e)
  {
    PE_ERROR(THISLOG << "exception occurried while executing command: '" << e.what() << "'.");
    throw;
  }
}

size_t Redis::removex(const string& pattern, unsigned batch) const
{
  size_t found = 0;
  size_t removed = 0;

  size_t loopGuard = 0;

  Cursor cursor;
  size_t prev = 0;
  do
  {
    prev = removed;
    auto keys = scan(cursor, pattern, batch);

    if (keys.size()>0)
    {
      loopGuard = 0;

      found += keys.size();
      removed += remove(keys);

      if (removed!=prev)
        cursor.reset(); //we must reset cursor after remove command
    }
    else
    {
      //it is not an error: commands are also allowed to return zero elements,
      //                    and the client should not consider the iteration complete
      //                    as long as the returned cursor is not zero
      if (++loopGuard>1000)
        throw exceptions::runtime_error("redis seems to have entered an endless loop");
    }

  } while(cursor || removed!=prev);

  if (found!=removed)
  {
    if (removed)
    {
      PE_WARNING(THISLOG << "only " << removed << " keys removed from " << found << " keys found.");
    }
    else
    {
      PE_WARNING(THISLOG << "No keys removed from " << found << " keys found.");
    }
  }

  return removed;
}

size_t Redis::remove(const string_list_t& keys) const
{
  if (keys.empty())
    return 0;

  static const string DEL("DEL");
  Command cmd(DEL);

  for (const auto& k: keys)
    cmd << k;

  size_t count = getInt<size_t>(exec(cmd));

  if (count!=keys.size())
  {
    if (count)
    {
      PE_WARNING(THISLOG << "only " << count << " keys removed from " << keys.size() << " keys.");
    }
    else
    {
      PE_WARNING(THISLOG << "No keys removed from " << keys.size() << " keys.");
    }
  }

  return count;
}

string_list_t Redis::scan(Command& cmd, Cursor& cursor, const string& pattern, unsigned limit) const
{
  cmd << cursor;

  if (!pattern.empty())
  {
    static const string MATCH("MATCH");
    cmd << MATCH << pattern;
  }

  if (limit>0)
  {
    static const string COUNT("COUNT");
    cmd << COUNT << limit;
  }

  auto r = exec(cmd);

  cursor.reset(); //will stop or restart next scan in case of errors

  string_list_t res;
  if (isArray(r))
  {
    if (r.elements().size()>=2)
    {
      cursor = r.elements()[0];
      const auto& keys = r.elements()[1].elements();
      for(const auto& k: keys)
      {
        if (!k.str().empty())
          res.push_back(k.str());
      }
    }
    else
    {
      PE_ERROR(THISLOG << "unexpected element count in result for SCAN command");
    }
  }
  else
  {
    PE_ERROR(THISLOG << "unexpected result type for SCAN command");
  }

  return res;
}

string_list_t Redis::get(const string_list_t& keys) const
{
  string_list_t result;

  if (keys.empty())
    return result;

  static const string MGET("MGET");
  Command cmd(MGET);

  for (const auto& k: keys)
    cmd << k;

  auto r = exec(cmd);

  if (isArray(r))
  {
    for (const auto& e: r.elements())
    {
      //if (!isNULL(e))
        result.push_back(e.str());
    }
  }
  else
  {
    PE_ERROR(THISLOG << "unexpected result type for MGET command");
  }

  return result;
}

} } //namespace pe { namespace redis {
