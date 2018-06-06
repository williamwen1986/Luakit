// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_X11_X11_ERROR_TRACKER_H_
#define BASE_X11_X11_ERROR_TRACKER_H_

#include <X11/Xlib.h>

#include "base/base_export.h"
#include "base/basictypes.h"

namespace base {

// X11ErrorTracker catches X11 errors in a non-fatal way. It does so by
// temporarily changing the X11 error handler. The old error handler is
// restored when the tracker is destroyed.
class BASE_EXPORT X11ErrorTracker {
 public:
  X11ErrorTracker();
  ~X11ErrorTracker();

  // Returns whether an X11 error happened since this function was last called
  // (or since the creation of the tracker). This is potentially expensive,
  // since this causes a sync with the X server.
  bool FoundNewError();

 private:
#if !defined(TOOLKIT_GTK)
  XErrorHandler old_handler_;
#endif

  DISALLOW_COPY_AND_ASSIGN(X11ErrorTracker);
};

}  // namespace base

#endif  // BASE_X11_X11_ERROR_TRACKER_H_
