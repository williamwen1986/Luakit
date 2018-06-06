// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/x11/x11_error_tracker.h"

#include "base/message_loop/message_pump_x11.h"

namespace {

unsigned char g_x11_error_code = 0;

int X11ErrorHandler(Display* display, XErrorEvent* error) {
  g_x11_error_code = error->error_code;
  return 0;
}

}

namespace base {

X11ErrorTracker::X11ErrorTracker() {
  old_handler_ = XSetErrorHandler(X11ErrorHandler);
}

X11ErrorTracker::~X11ErrorTracker() {
  XSetErrorHandler(old_handler_);
}

bool X11ErrorTracker::FoundNewError() {
  XSync(MessagePumpForUI::GetDefaultXDisplay(), False);
  unsigned char error = g_x11_error_code;
  g_x11_error_code = 0;
  return error != 0;
}

}  // namespace ui
