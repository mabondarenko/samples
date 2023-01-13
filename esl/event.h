#ifndef _PE_ESL_EVENT__H_20200416_
#define _PE_ESL_EVENT__H_20200416_

#include "ievent.h"

namespace pe { namespace esl {

class Event : public IEvent
{
public:
  explicit Event(const string& _from, const char* _contentType);
  explicit Event(const json& _from, const char* _contentType);

public:
  virtual const json& body() const override;
  virtual string bodyStr() const override;
  virtual const string& contentType() const override { return contentType_; }
  virtual bool isAPIResponse() const override;
  virtual bool isCommandReply() const override;
  virtual bool isDisconnectNotice() const override;
  virtual bool isErrorResponse() const override;
  virtual const string& name() const override { return name_; }
  virtual const json& toJson() const override { return data_; }
  virtual string toString(int indent=2) const override { return toJson().dump(indent); }

protected:
  string value(const char* field, const string& def=string()) const;
  bool checkError() const;

private:
  json data_;
  string name_;
  string contentType_;
  mutable bool errorDetected_ = false;
  mutable bool errorFlag_ = false;
};

} } //namespace pe { namespace esl {

#endif //#ifndef _PE_ESL_EVENT__H_20200416_