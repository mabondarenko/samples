#ifndef _PE_SCGI_RESPONSE__20190305_
#define _PE_SCGI_RESPONSE__20190305_

#include "types.h"

#include "reqbase.h"

namespace pe { namespace scgi {

class Response : public RequestBase
{
  using _Base = RequestBase;

public:
  Response(const RequestBase& from, const json& data=json());
  Response(const RequestBase& from, int code, const string& status);

public:
  int code() const { return code_; }
  const string& status() const { return status_; }

public:
  virtual json toJson() const override;
  virtual void fromJson(const json& from) override;

protected:
  void init(const RequestBase& from);

  void set_code(int value) { code_ = value; }
  void set_status(const string& value) { status_ = value; }

private:
  int code_;
  string status_;
};

} } //namespace pe { namespace scgi {

#endif //#ifndef _PE_SCGI_RESPONSE__20190305_
