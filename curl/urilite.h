#ifndef _PE_API_CURL_URILITE__H_20190405_
#define _PE_API_CURL_URILITE__H_20190405_

#include "types.h"

#include <sstream>
#include <iomanip>

namespace pe { namespace urilite {

class PLUS : public noncopyable {
 public:
  static string space_encoded() { return "+"; }
  static char plus_decoded() { return ' '; }
};

class NO_PLUS : public noncopyable {
 public:
  static string space_encoded() { return "%20"; }
  static char plus_decoded() { return '+'; }
};

class RFC2396 : public noncopyable {
public:
  static bool match(const char c) {
    // unreserved?
    if ((c >= 'a' && c <= 'z')
     || (c >= 'A' && c <= 'Z')
     || (c >= '0' && c <= '9')
     || c == '-'  || c == '_'  || c == '.' || c == '~'
     || c == '!'  || c == '\'' || c == '(' || c == ')')
      return false;
    else
      return true;
  }
};

class RFC3986 : public noncopyable {
public:
  static bool match(const char c) {
    // unreserved?
    if ((c >= 'a' && c <= 'z')
     || (c >= 'A' && c <= 'Z')
     || (c >= '0' && c <= '9')
     || c == '-' || c == '_' || c == '.' || c == '~')
      return false;
    else
      return true;
  }
};

template<typename T = RFC3986, typename U = NO_PLUS>
class encoder : public noncopyable {
public:
    static string encode(const string& s) {
        std::ostringstream oss;
        for (size_t i=0; i < s.size(); ++i) {
            char c = s[i];
            if (!T::match(c)) {
                oss << c;
            } else if (c == ' ') {
                oss << U::space_encoded();
            } else {
                oss << '%' << std::hex << std::setw(2) << std::setfill('0') << (c & 0xff) << std::dec;
            }
        }
        return oss.str();
    }

    static string decode(const string& s) {
        std::ostringstream oss;
        for (size_t i = 0; i < s.size(); ++i) {
            char c = s[i];
            if (c == '+') {
                oss << U::plus_decoded();
            } else if (c == '%') {
                int d;
                std::istringstream iss(s.substr(i+1, 2));
                iss >> std::hex >> d;
                oss << static_cast<char>(d);
                i += 2;
            } else {
                oss << c;
            }
        }
        return oss.str();
    }
}; // end of class

} }  // namespace pe { namespace urilite {

#endif //#ifndef _PE_API_CURL_URILITE__H_20190405_
