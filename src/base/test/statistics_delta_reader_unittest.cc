// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/memory/scoped_ptr.h"
#include "base/metrics/histogram.h"
#include "base/metrics/histogram_samples.h"
#include "base/metrics/statistics_recorder.h"
#include "base/test/statistics_delta_reader.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace base {

class StatisticsDeltaReaderTest : public testing::Test {
 protected:
  virtual void SetUp() OVERRIDE {
    // Each test will have a clean state (no Histogram / BucketRanges
    // registered).
    statistics_recorder_ = new StatisticsRecorder();
  }

  virtual void TearDown() OVERRIDE {
    delete statistics_recorder_;
    statistics_recorder_ = NULL;
  }

  StatisticsRecorder* statistics_recorder_;
};

TEST_F(StatisticsDeltaReaderTest, Scope) {
  // Record a histogram before the creation of the recorder.
  UMA_HISTOGRAM_BOOLEAN("Test", true);

  StatisticsDeltaReader reader;

  // Verify that no histogram is recorded.
  scoped_ptr<HistogramSamples> samples(
      reader.GetHistogramSamplesSinceCreation("Test"));
  EXPECT_TRUE(samples);
  EXPECT_EQ(0, samples->TotalCount());

  // Record a histogram after the creation of the recorder.
  UMA_HISTOGRAM_BOOLEAN("Test", true);

  // Verify that one histogram is recorded.
  samples = reader.GetHistogramSamplesSinceCreation("Test");
  EXPECT_TRUE(samples);
  EXPECT_EQ(1, samples->TotalCount());
}

}  // namespace base
