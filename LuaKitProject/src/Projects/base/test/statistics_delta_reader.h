// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TEST_STATISTICS_DELTA_READER_H_
#define BASE_TEST_STATISTICS_DELTA_READER_H_

#include <map>
#include <string>

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "base/metrics/histogram_samples.h"

namespace base {

// This class acts as a differential reader for histogram samples, enabling
// tests to check that metrics were recorded as they should be.
// Before using this class, StatisticsRecoder must be initialized.
class StatisticsDeltaReader {
 public:
  StatisticsDeltaReader();
  ~StatisticsDeltaReader();

  // Returns the histogram data accumulated since this instance was created.
  // Returns NULL if no samples are available.
  scoped_ptr<HistogramSamples> GetHistogramSamplesSinceCreation(
      const std::string& histogram_name);

 private:
  // Used to determine the histogram changes made during this instance's
  // lifecycle. This instance takes ownership of the samples, which are deleted
  // when the instance is destroyed.
  std::map<std::string, HistogramSamples*> original_samples_;

  DISALLOW_COPY_AND_ASSIGN(StatisticsDeltaReader);
};

}  // namespace base

#endif  // BASE_TEST_STATISTICS_DELTA_READER_H_
