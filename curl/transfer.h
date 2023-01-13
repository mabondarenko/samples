#ifndef _PE_CURL_TRANSFER__H_20190917_
#define _PE_CURL_TRANSFER__H_20190917_

#include "easy.h"

#include <functional>

namespace pe { namespace curl {

class Service;
class Url;

class Transfer : public Easy
               , public std::enable_shared_from_this<Transfer>
{
  using _Base = Easy;

public:
  using pointer_t = std::shared_ptr<Transfer>;
  using ServicePtr = std::shared_ptr<Service>;
  using ResultHandler = std::function<void(const pointer_t& transfer,
                                           int error,
                                           const char* text)>;
  using ReadHandler = std::function<size_t(const pointer_t& transfer,
                                           char* buf,
                                           size_t size)>;
  using WriteHandler = std::function<size_t(const pointer_t& transfer,
                                            const char* buf,
                                            size_t size)>;

public:
  Transfer(const ServicePtr& service,
           const string& url=string(),
           bool verbose=false,
           uint64_t timeout=REQUEST_TIMEOUT_MS);
  virtual ~Transfer() override;

public:
  virtual void exec(ResultHandler&& handler);

public:
  void read(ReadHandler&& handler) { read_ = handler; }
  void write(WriteHandler&& handler) { write_ = handler; }

  Url getUrl() const;
  const ServicePtr& service() const { return service_; }
  void loginOptions(const string& opt) const { return loginOptions(opt.c_str()); }
  void loginOptions(const char* opt) const { set(CURLOPT_LOGIN_OPTIONS, opt); }
  void keyPassword(const string& passw) const { return keyPassword(passw.c_str()); }
  void keyPassword(const char* passw) const { set(CURLOPT_KEYPASSWD, passw); }
  void noBODY(bool value=true) const { set(CURLOPT_NOBODY, value); }
  void sshPrivateKey(const string& file) const { return sshPrivateKey(file.c_str()); }
  void sshPrivateKey(const char* file) const { set(CURLOPT_SSH_PRIVATE_KEYFILE, file); }
  void sshPublicKey(const string& file) const { return sshPublicKey(file.c_str()); }
  void sshPublicKey(const char* file) const { set(CURLOPT_SSH_PUBLIC_KEYFILE, file); }
  string url() const;
  void url(const string& _url) { return url(_url.c_str()); }
  void url(const char* _url) { set(CURLOPT_URL, _url); }
  void userName(const string& user) const { return userName(user.c_str()); }
  void userName(const char* user) const { set(CURLOPT_USERNAME, user); }
  void userPassword(const string& passw) const { return userPassword(passw.c_str()); }
  void userPassword(const char* passw) const { set(CURLOPT_PASSWORD, passw); }
  void quote(const StringList& commands) const { return set(CURLOPT_QUOTE, commands); }

protected:
  void attach();
  void detach();
  void reset();

protected:
  virtual void done(int error, const char* text);
  virtual void dispatch(const CURLMsg* msg) override;
  virtual void debug(curl_infotype type, const char* data, size_t size) override;
  virtual int progress(curl_off_t downloadTotal,
                       curl_off_t downloaded,
                       curl_off_t uploadTotal,
                       curl_off_t uploaded) override;
  virtual size_t read(char* buf, size_t size) override;
  virtual size_t write(const char* buf, size_t size) override;
  virtual void canceling() override;

private:
  ServicePtr service_;
  ResultHandler done_;
  ReadHandler read_;
  WriteHandler write_;
  bool attached_ = false;
};

} } //namespace pe { namespace curl {

#endif //#ifndef _PE_CURL_TRANSFER__H_20190917_