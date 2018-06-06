// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMMON_ANDROID_ALARM_TICK_CLOCK_H_
#define COMMON_ANDROID_ALARM_TICK_CLOCK_H_

#include "base/base_export.h"
#include "base/compiler_specific.h"
#include "base/time/tick_clock.h"

namespace base {

// AlarmClock is a Clock implementation that uses TimeTicks::Now() in common platform, but uses alarm clock on Android platform instead.
class BASE_EXPORT AlarmTickClock : public TickClock {
 public:
  virtual ~AlarmTickClock();

  virtual TimeTicks NowTicks() OVERRIDE;
};

}  // namespace base

#endif  // COMMON_ANDROID_ALARM_TICK_CLOCK_H_
