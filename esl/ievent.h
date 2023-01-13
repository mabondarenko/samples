#ifndef _PE_ESL_IEVENT__H_20200416_
#define _PE_ESL_IEVENT__H_20200416_

#include "types.h"

#include <nlohmann/json.hpp>

namespace pe { namespace esl {

struct IEvent
{
  using json = nlohmann::json;
  using pointer_t = std::shared_ptr<IEvent>;

  virtual ~IEvent()=default;

  virtual const json& body() const=0;
  virtual string bodyStr() const=0;
  virtual const string& contentType() const=0;
  virtual bool isAPIResponse() const=0;
  virtual bool isCommandReply() const=0;
  virtual bool isDisconnectNotice() const=0;
  virtual bool isErrorResponse() const=0;
  virtual const string& name() const=0;
  virtual const json& toJson() const=0;
  virtual string toString(int indent=2) const=0;
};

} } //namespace pe { namespace esl {

#endif //#ifndef _PE_ESL_IEVENT__H_20200416_