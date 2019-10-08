// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_BASE_NET_ERRORS_H__
#define NET_BASE_NET_ERRORS_H__

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/files/file.h"
#include "base/logging.h"

namespace net {

// Error domain of the net module's error codes.
extern const char kErrorDomain[];

// Error values are negative.
enum Error {
  // No error.
  OK = 0,

#define NET_ERROR(label, value) ERR_ ## label = value,
#include "net_error_list.h"
#undef NET_ERROR

  // The value of the first certificate error code.
  ERR_CERT_BEGIN = ERR_CERT_COMMON_NAME_INVALID,
};

// Returns a textual representation of the error code for logging purposes.
std::string ErrorToString(int error);

// Same as above, but leaves off the leading "net::".
std::string ErrorToShortString(int error);

// Returns true if |error| is a certificate error code.
bool IsCertificateError(int error);

// Returns true if |error| is a client certificate authentication error. This
// does not include ERR_SSL_PROTOCOL_ERROR which may also signal a bad client
// certificate.
bool IsClientCertificateError(int error);

// Map system error code to Error.
Error MapSystemError(logging::SystemErrorCode os_error);

// A convenient function to translate file error to net error code.
Error FileErrorToNetError(base::File::Error file_error);

}  // namespace net

#endif  // NET_BASE_NET_ERRORS_H__
