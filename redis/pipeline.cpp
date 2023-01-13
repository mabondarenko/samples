#include "pipeline.h"

#include "trace.h"

namespace pe { namespace redis {

PE_DECLARE_LOG_CHANNEL(THISLOG, "redis.pipeline: ");

Pipeline& Pipeline::append(const Command& cmd)
{
  tr_.push_back(cmd);
  return *this;
}

Pipeline& Pipeline::append(Command&& cmd)
{
  tr_.emplace_back(cmd);
  return *this;
}

ReplySet Pipeline::exec() const
{
  try
  {
    return pool()->run_with_connection<ReplySet>([&](redis3m::connection::ptr_t conn)
    {
      for (const auto& cmd: tr())
      {
        if (trace())
        {
          PE_TRACE(THISLOG << debugString(cmd));
        }
        conn->append(args(cmd));
      }

      return conn->get_replies(size());
    },
    retries());
  }
  catch(std::exception& e)
  {
    PE_ERROR(THISLOG << "exception occurried while pipe commands: '" << e.what() << "'.");
    throw;
  }
}

} } //namespace pe { namespace redis {
