// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/alarm_tick_clock.h"
#include "base/logging.h"
#include <sys/ioctl.h>
#include <fcntl.h>

namespace base {

AlarmTickClock::~AlarmTickClock() {}

TimeTicks AlarmTickClock::NowTicks() {
  return TimeTicks::Now();
}

}  // namespace base
