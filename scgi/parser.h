#ifndef _PE_SCGI_PARSER__20190228_
#define _PE_SCGI_PARSER__20190228_

#include "types.h"

#include "message.h"

namespace pe { namespace scgi {

class Connection;
class Dispatcher;

class Parser : public std::enable_shared_from_this<Parser>
             , noncopyable
{
  using ConnectionPtr = std::shared_ptr<Connection>;
  using DispatcherPtr = std::shared_ptr<Dispatcher>;
  using MessageHeader = Message::Header;

public:
  Parser(const DispatcherPtr& dispatcher);
  virtual ~Parser();

public:
  void read(const ConnectionPtr& connection);

protected:
  bool process(const ConnectionPtr& connection, const buffer& buf, size_t size);

private:
  DispatcherPtr dispatcher_;
  buffer rdbuf_;
  buffer msgbuf_;
  MessageHeader header_;
  bool failed_ = false;
  bool skipmsg_ = false;
  size_t msgsize_ = 0;
  size_t nread_ = 0;
};

} } //namespace pe { namespace scgi {

#endif //#ifndef _PE_SCGI_PARSER__20190228_
