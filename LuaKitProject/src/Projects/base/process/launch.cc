// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/process/launch.h"

namespace base {

LaunchOptions::LaunchOptions()
    : wait(false),
#if defined(OS_WIN)
      start_hidden(false),
      handles_to_inherit(NULL),
      inherit_handles(false),
      as_user(NULL),
      empty_desktop_name(false),
      job_handle(NULL),
      stdin_handle(NULL),
      stdout_handle(NULL),
      stderr_handle(NULL),
      force_breakaway_from_job_(false)
#else
      fds_to_remap(NULL),
      maximize_rlimits(NULL),
      new_process_group(false)
#if defined(OS_LINUX)
      , clone_flags(0)
#endif  // OS_LINUX
#if defined(OS_CHROMEOS)
      , ctrl_terminal_fd(-1)
#endif  // OS_CHROMEOS
#endif  // !defined(OS_WIN)
    {
}

LaunchOptions::~LaunchOptions() {
}

}  // namespace base
