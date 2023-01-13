#ifndef _PE_SCGI_REQBASE__20190305_
#define _PE_SCGI_REQBASE__20190305_

#include "types.h"

#include <nlohmann/json.hpp>

namespace pe { namespace scgi {

class RequestBase : public std::enable_shared_from_this<RequestBase>
                  , noncopyable
{
protected:
  using json = nlohmann::json;

public:
  using reqid_t = uint32_t;
  enum class Method
  {
    kUNKNOWN,
    kGET,
    kPOST,
    kPUT,
    kPATCH,
    kDELETE,
    kEVENT
  };

  struct Param
  {
    Param(const string& uri);

    string param;
    string item;
  };

public:
  RequestBase()=default;
  virtual ~RequestBase()=default;

public:
  const reqid_t& id() const { return id_; }
  const Method& method() const { return method_; }
  string method_name() const { return enums::toString(method_map(), method()); }
  const string& uri() const { return uri_; }
  const string& sessionId() const { return sessionId_; }
  const json& data() const { return data_; }
  Param param() const;

public:
  string toString() const { return toJson().dump(); }

public:
  virtual json toJson() const;
  virtual void fromJson(const json& from);

protected:
  void set_id(const reqid_t& value) { id_ = value; }
  void set_method(const Method& value) { method_ = value; }
  void set_method(const string& value) { set_method(enums::toEnum(value, method_map())); }
  void set_uri(const string& value);
  void set_sessionId(const string& value) { sessionId_ = value; }
  void set_data(const json& value) { data_ = value; }

  using method_map_t = enums::map<Method>;
  static const method_map_t& method_map();
  static bool present(const json& j, const char* name) { return j.find(name)!=j.end(); }

private:
  reqid_t id_ = 0;
  Method method_ = Method::kUNKNOWN;
  string uri_;
  string sessionId_;
  json data_;
};

} } //namespace pe { namespace scgi {

#endif //#ifndef _PE_SCGI_REQBASE__20190305_
