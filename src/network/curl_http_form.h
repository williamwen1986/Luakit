#ifndef curl_http_form_h
#define curl_http_form_h

#include <string>
#include "base/macros.h"

// forward delcares
struct curl_httppost;

namespace network {

class CurlHttpForm {
 public:
  CurlHttpForm();

  virtual ~CurlHttpForm();

  void AddFileSection(const std::string& name, const std::string& path, const std::string& mime = std::string());

  void AddStringSection(const std::string& field, const std::string& value);

  struct curl_httppost* GetForm() const;

 private:
  struct curl_httppost* formpost_;
  struct curl_httppost *lastptr_;

  DISALLOW_ASSIGN(CurlHttpForm);
}; // end of class class CurlHttpForm

} // end of namespace network

#endif /* curl_http_form_h */
