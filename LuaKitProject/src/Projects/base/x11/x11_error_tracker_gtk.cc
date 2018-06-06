// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/x11/x11_error_tracker.h"

#include <gdk/gdkx.h>

#include "base/logging.h"

namespace base {

X11ErrorTracker::X11ErrorTracker() {
  gdk_error_trap_push();
}

X11ErrorTracker::~X11ErrorTracker() {
  gdk_error_trap_pop();
}

bool X11ErrorTracker::FoundNewError() {
  gdk_flush();
  bool found_error = gdk_error_trap_pop() != 0;

  gdk_error_trap_push();
  return found_error;
}

}  // namespace base
