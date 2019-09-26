// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/fuchsia/filtered_service_directory.h"

#include <lib/async/default.h>
#include <utility>

#include "base/bind.h"
#include "base/fuchsia/fuchsia_logging.h"

namespace base {
namespace fuchsia {

FilteredServiceDirectory::FilteredServiceDirectory(
    sys::ServiceDirectory* directory)
    : directory_(std::move(directory)) {
  outgoing_directory_.Serve(
      outgoing_directory_client_.NewRequest().TakeChannel());
}

FilteredServiceDirectory::~FilteredServiceDirectory() {}

void FilteredServiceDirectory::AddService(const char* service_name) {
  outgoing_directory_.AddPublicService(
      std::make_unique<vfs::Service>(
          [this, service_name](zx::channel channel,
                               async_dispatcher_t* dispatcher) {
            DCHECK_EQ(dispatcher, async_get_default_dispatcher());
            directory_->Connect(service_name, std::move(channel));
          }),
      service_name);
}

void FilteredServiceDirectory::ConnectClient(
    fidl::InterfaceRequest<::fuchsia::io::Directory> dir_request) {
  // sys::OutgoingDirectory puts public services under ./svc . Connect to that
  // directory and return client handle for the connection,
  outgoing_directory_.GetOrCreateDirectory("svc")->Serve(
      ::fuchsia::io::OPEN_RIGHT_READABLE | ::fuchsia::io::OPEN_RIGHT_WRITABLE,
      dir_request.TakeChannel());
}

}  // namespace fuchsia
}  // namespace base
