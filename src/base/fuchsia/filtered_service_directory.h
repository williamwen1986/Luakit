// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_FUCHSIA_FILTERED_SERVICE_DIRECTORY_H_
#define BASE_FUCHSIA_FILTERED_SERVICE_DIRECTORY_H_

#include <fuchsia/io/cpp/fidl.h>
#include <lib/fidl/cpp/interface_handle.h>
#include <lib/sys/cpp/outgoing_directory.h>
#include <lib/sys/cpp/service_directory.h>
#include <lib/zx/channel.h>
#include <memory>

#include "base/base_export.h"
#include "base/macros.h"

namespace base {
namespace fuchsia {

// ServiceDirectory that uses the supplied ServiceDirectoryClient to satisfy
// requests for only a restricted set of services.
class BASE_EXPORT FilteredServiceDirectory {
 public:
  // Creates a directory that proxies requests to the specified service
  // |directory|.
  explicit FilteredServiceDirectory(sys::ServiceDirectory* directory);
  ~FilteredServiceDirectory();

  // Adds the specified service to the list of whitelisted services.
  void AddService(const char* service_name);

  // Connects a directory client. The directory can be passed to a sandboxed
  // process to be used for /svc namespace.
  void ConnectClient(
      fidl::InterfaceRequest<::fuchsia::io::Directory> dir_request);

 private:
  const sys::ServiceDirectory* const directory_;
  sys::OutgoingDirectory outgoing_directory_;

  // Client side of the channel used by |outgoing_directory_|.
  fidl::InterfaceHandle<::fuchsia::io::Directory> outgoing_directory_client_;

  DISALLOW_COPY_AND_ASSIGN(FilteredServiceDirectory);
};

}  // namespace fuchsia
}  // namespace base

#endif  // BASE_FUCHSIA_FILTERED_SERVICE_DIRECTORY_H_
