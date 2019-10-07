// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/util/memory_pressure/multi_source_memory_pressure_monitor.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/metrics/histogram_macros.h"
#include "base/util/memory_pressure/system_memory_pressure_evaluator.h"

namespace util {

MultiSourceMemoryPressureMonitor::MultiSourceMemoryPressureMonitor()
    : current_pressure_level_(
          base::MemoryPressureListener::MEMORY_PRESSURE_LEVEL_NONE),
      dispatch_callback_(base::BindRepeating(
          &base::MemoryPressureListener::NotifyMemoryPressure)),
      aggregator_(this) {
  // This can't be in the parameter list because |sequence_checker_| wouldn't be
  // available, which would be needed by the |system_evaluator_|'s constructor's
  // call to CreateVoter().
  system_evaluator_ =
      SystemMemoryPressureEvaluator::CreateDefaultSystemEvaluator(this);
  StartMetricsTimer();
}

MultiSourceMemoryPressureMonitor::~MultiSourceMemoryPressureMonitor() = default;

void MultiSourceMemoryPressureMonitor::StartMetricsTimer() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  // Unretained is safe here since this task is running on a timer owned by this
  // object.
  metric_timer_.Start(
      FROM_HERE, MemoryPressureMonitor::kUMAMemoryPressureLevelPeriod,
      BindRepeating(
          &MultiSourceMemoryPressureMonitor::RecordCurrentPressureLevel,
          base::Unretained(this)));
}

void MultiSourceMemoryPressureMonitor::StopMetricsTimer() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  metric_timer_.Stop();
}

void MultiSourceMemoryPressureMonitor::RecordCurrentPressureLevel() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  RecordMemoryPressure(GetCurrentPressureLevel(), /* ticks = */ 1);
}

base::MemoryPressureListener::MemoryPressureLevel
MultiSourceMemoryPressureMonitor::GetCurrentPressureLevel() const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return current_pressure_level_;
}

std::unique_ptr<MemoryPressureVoter>
MultiSourceMemoryPressureMonitor::CreateVoter() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return std::make_unique<MemoryPressureVoter>(&aggregator_);
}

void MultiSourceMemoryPressureMonitor::SetDispatchCallback(
    const DispatchCallback& callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  dispatch_callback_ = callback;
}

void MultiSourceMemoryPressureMonitor::OnMemoryPressureLevelChanged(
    base::MemoryPressureListener::MemoryPressureLevel level) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  current_pressure_level_ = level;
}

void MultiSourceMemoryPressureMonitor::OnNotifyListenersRequested() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  dispatch_callback_.Run(current_pressure_level_);
}

void MultiSourceMemoryPressureMonitor::ResetSystemEvaluatorForTesting() {
  system_evaluator_.reset();
}

void MultiSourceMemoryPressureMonitor::SetSystemEvaluator(
    std::unique_ptr<SystemMemoryPressureEvaluator> evaluator) {
  DCHECK(!system_evaluator_);
  system_evaluator_ = std::move(evaluator);
}

}  // namespace util
