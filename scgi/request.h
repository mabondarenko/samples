#ifndef _PE_SCGI_REQUEST__20190305_
#define _PE_SCGI_REQUEST__20190305_

#include "types.h"

#include "reqbase.h"

namespace pe { namespace scgi {

class Connection;

class Request : public RequestBase
{
  using _Base = RequestBase;
  using ConnectionPtr = std::shared_ptr<Connection>;
  using ConnectionWeakPtr = std::weak_ptr<Connection>;

public:
  using pointer = std::shared_ptr<Request>;

public:
  Request(const ConnectionPtr& connection, const buffer& msg);

public:
  const string& from() const { return from_; }

public:
  void reply(int code, const string& status);
  void reply(const json& data=json());

public:
  virtual json toJson() const override;
  virtual void fromJson(const json& from) override;

protected:
  void set_from(const string& value) { from_ = value; }

protected:
  ConnectionPtr connection() const { return connection_.lock(); }

private:
  ConnectionWeakPtr connection_;
  string from_;
};

} } //namespace pe { namespace scgi {

#endif //#ifndef _PE_SCGI_REQUEST__20190305_
