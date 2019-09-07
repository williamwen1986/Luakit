#include "curl_http_form.h"
#include "curl/curl.h"
//#include <zlib.h>

namespace network {

CurlHttpForm::CurlHttpForm()
  : formpost_(nullptr),
    lastptr_(nullptr) {
}

CurlHttpForm::~CurlHttpForm() {
  if (formpost_) {
    curl_formfree(formpost_);
  }
}

void CurlHttpForm::AddFileSection(const std::string &name, const std::string &path, const std::string& mime) {
  if (mime.empty()) {
    curl_formadd(
      &formpost_,
      &lastptr_,
      CURLFORM_COPYNAME, name.c_str(),
      CURLFORM_FILE, path.c_str(),
      CURLFORM_FILENAME, "log.gz",
      CURLFORM_END);
  } else {
    curl_formadd(
      &formpost_,
      &lastptr_,
      CURLFORM_COPYNAME, name.c_str(),
      CURLFORM_FILE, path.c_str(),
      CURLFORM_CONTENTTYPE, mime.c_str(),
      CURLFORM_END);
  }
}

void CurlHttpForm::AddStringSection(const std::string &field, const std::string &value) {
  curl_formadd(
    &formpost_,
    &lastptr_,
    CURLFORM_COPYNAME, field.c_str(),
    CURLFORM_COPYCONTENTS, value.c_str(),
    CURLFORM_END);
}

struct curl_httppost* CurlHttpForm::GetForm() const {
  return formpost_;
}

} // end of namespace network

