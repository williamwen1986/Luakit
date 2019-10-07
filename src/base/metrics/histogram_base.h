// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_METRICS_HISTOGRAM_BASE_H_
#define BASE_METRICS_HISTOGRAM_BASE_H_

#include <string>
#include <vector>

#include "base/atomicops.h"
#include "base/base_export.h"
#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "base/time/time.h"

class Pickle;
class PickleIterator;

namespace base {

class DictionaryValue;
class HistogramBase;
class HistogramSamples;
class ListValue;

////////////////////////////////////////////////////////////////////////////////
// These enums are used to facilitate deserialization of histograms from other
// processes into the browser. If you create another class that inherits from
// HistogramBase, add new histogram types and names below.

enum BASE_EXPORT HistogramType {
  HISTOGRAM,
  LINEAR_HISTOGRAM,
  BOOLEAN_HISTOGRAM,
  CUSTOM_HISTOGRAM,
  SPARSE_HISTOGRAM,
};

std::string HistogramTypeToString(HistogramType type);

// Create or find existing histogram that matches the pickled info.
// Returns NULL if the pickled data has problems.
BASE_EXPORT_PRIVATE HistogramBase* DeserializeHistogramInfo(
    PickleIterator* iter);

////////////////////////////////////////////////////////////////////////////////

class BASE_EXPORT HistogramBase {
 public:
  typedef int Sample;                   // Used for samples.
  typedef subtle::Atomic32 Count;     // Used to count samples.

  static const Sample kSampleType_MAX;  // INT_MAX

  enum Flags {
    kNoFlags = 0,
    kUmaTargetedHistogramFlag = 0x1,  // Histogram should be UMA uploaded.

    // Indicate that the histogram was pickled to be sent across an IPC Channel.
    // If we observe this flag on a histogram being aggregated into after IPC,
    // then we are running in a single process mode, and the aggregation should
    // not take place (as we would be aggregating back into the source
    // histogram!).
    kIPCSerializationSourceFlag = 0x10,

    // Only for Histogram and its sub classes: fancy bucket-naming support.
    kHexRangePrintingFlag = 0x8000,
  };

  // Histogram data inconsistency types.
  enum Inconsistency {
    NO_INCONSISTENCIES = 0x0,
    RANGE_CHECKSUM_ERROR = 0x1,
    BUCKET_ORDER_ERROR = 0x2,
    COUNT_HIGH_ERROR = 0x4,
    COUNT_LOW_ERROR = 0x8,

    NEVER_EXCEEDED_VALUE = 0x10
  };

  explicit HistogramBase(const std::string& name);
  virtual ~HistogramBase();

  std::string histogram_name() const { return histogram_name_; }

  // Operations with Flags enum.
  int32 flags() const { return flags_; }
  void SetFlags(int32 flags);
  void ClearFlags(int32 flags);

  virtual HistogramType GetHistogramType() const = 0;

  // Whether the histogram has construction arguments as parameters specified.
  // For histograms that don't have the concept of minimum, maximum or
  // bucket_count, this function always returns false.
  virtual bool HasConstructionArguments(Sample expected_minimum,
                                        Sample expected_maximum,
                                        size_t expected_bucket_count) const = 0;

  virtual void Add(Sample value) = 0;

  // 2 convenient functions that call Add(Sample).
  void AddTime(const TimeDelta& time);
  void AddBoolean(bool value);

  virtual void AddSamples(const HistogramSamples& samples) = 0;
  virtual bool AddSamplesFromPickle(PickleIterator* iter) = 0;

  // Serialize the histogram info into |pickle|.
  // Note: This only serializes the construction arguments of the histogram, but
  // does not serialize the samples.
  bool SerializeInfo(Pickle* pickle) const;

  // Try to find out data corruption from histogram and the samples.
  // The returned value is a combination of Inconsistency enum.
  virtual int FindCorruption(const HistogramSamples& samples) const;

  // Snapshot the current complete set of sample data.
  // Override with atomic/locked snapshot if needed.
  virtual scoped_ptr<HistogramSamples> SnapshotSamples() const = 0;

  // The following methods provide graphical histogram displays.
  virtual void WriteHTMLGraph(std::string* output) const = 0;
  virtual void WriteAscii(std::string* output) const = 0;

  // Produce a JSON representation of the histogram. This is implemented with
  // the help of GetParameters and GetCountAndBucketData; overwrite them to
  // customize the output.
//  void WriteJSON(std::string* output) const;

protected:
  // Subclasses should implement this function to make SerializeInfo work.
  virtual bool SerializeInfoImpl(Pickle* pickle) const = 0;

  // Writes information about the construction parameters in |params|.
  virtual void GetParameters(DictionaryValue* params) const = 0;

  // Writes information about the current (non-empty) buckets and their sample
  // counts to |buckets|, the total sample count to |count| and the total sum
  // to |sum|.
  virtual void GetCountAndBucketData(Count* count,
                                     int64* sum,
                                     ListValue* buckets) const = 0;

  //// Produce actual graph (set of blank vs non blank char's) for a bucket.
  void WriteAsciiBucketGraph(double current_size,
                             double max_size,
                             std::string* output) const;

  // Return a string description of what goes in a given bucket.
  const std::string GetSimpleAsciiBucketRange(Sample sample) const;

  // Write textual description of the bucket contents (relative to histogram).
  // Output is the count in the buckets, as well as the percentage.
  void WriteAsciiBucketValue(Count current,
                             double scaled_sum,
                             std::string* output) const;

 private:
  const std::string histogram_name_;
  int32 flags_;

  DISALLOW_COPY_AND_ASSIGN(HistogramBase);
};

}  // namespace base

#endif  // BASE_METRICS_HISTOGRAM_BASE_H_
