// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/memory/discardable_memory_provider.h"

#include "base/bind.h"
#include "base/containers/hash_tables.h"
#include "base/containers/mru_cache.h"
#include "base/debug/trace_event.h"
#include "base/synchronization/lock.h"
#include "base/sys_info.h"

namespace base {
namespace internal {

namespace {

// This is admittedly pretty magical. It's approximately enough memory for two
// 2560x1600 images.
static const size_t kDefaultDiscardableMemoryLimit = 32 * 1024 * 1024;
static const size_t kDefaultBytesToReclaimUnderModeratePressure =
    kDefaultDiscardableMemoryLimit / 2;

}  // namespace

DiscardableMemoryProvider::DiscardableMemoryProvider()
    : allocations_(AllocationMap::NO_AUTO_EVICT),
      bytes_allocated_(0),
      discardable_memory_limit_(kDefaultDiscardableMemoryLimit),
      bytes_to_reclaim_under_moderate_pressure_(
          kDefaultBytesToReclaimUnderModeratePressure),
      memory_pressure_listener_(
          base::Bind(&DiscardableMemoryProvider::NotifyMemoryPressure,
                     Unretained(this))) {
}

DiscardableMemoryProvider::~DiscardableMemoryProvider() {
  DCHECK(allocations_.empty());
  DCHECK_EQ(0u, bytes_allocated_);
}

void DiscardableMemoryProvider::NotifyMemoryPressure(
    MemoryPressureListener::MemoryPressureLevel pressure_level) {
  switch (pressure_level) {
    case MemoryPressureListener::MEMORY_PRESSURE_MODERATE:
      Purge();
      return;
    case MemoryPressureListener::MEMORY_PRESSURE_CRITICAL:
      PurgeAll();
      return;
  }

  NOTREACHED();
}

void DiscardableMemoryProvider::SetDiscardableMemoryLimit(size_t bytes) {
  AutoLock lock(lock_);
  discardable_memory_limit_ = bytes;
  EnforcePolicyWithLockAcquired();
}

void DiscardableMemoryProvider::SetBytesToReclaimUnderModeratePressure(
    size_t bytes) {
  AutoLock lock(lock_);
  bytes_to_reclaim_under_moderate_pressure_ = bytes;
}

void DiscardableMemoryProvider::Register(
    const DiscardableMemory* discardable, size_t bytes) {
  AutoLock lock(lock_);
  DCHECK(allocations_.Peek(discardable) == allocations_.end());
  allocations_.Put(discardable, Allocation(bytes));
}

void DiscardableMemoryProvider::Unregister(
    const DiscardableMemory* discardable) {
  AutoLock lock(lock_);
  AllocationMap::iterator it = allocations_.Peek(discardable);
  if (it == allocations_.end())
    return;

  if (it->second.memory) {
    size_t bytes = it->second.bytes;
    DCHECK_LE(bytes, bytes_allocated_);
    bytes_allocated_ -= bytes;
    free(it->second.memory);
  }
  allocations_.Erase(it);
}

scoped_ptr<uint8, FreeDeleter> DiscardableMemoryProvider::Acquire(
    const DiscardableMemory* discardable,
    bool* purged) {
  AutoLock lock(lock_);
  // NB: |allocations_| is an MRU cache, and use of |Get| here updates that
  // cache.
  AllocationMap::iterator it = allocations_.Get(discardable);
  CHECK(it != allocations_.end());

  if (it->second.memory) {
    scoped_ptr<uint8, FreeDeleter> memory(it->second.memory);
    it->second.memory = NULL;
    *purged = false;
    return memory.Pass();
  }

  size_t bytes = it->second.bytes;
  if (!bytes)
    return scoped_ptr<uint8, FreeDeleter>();

  if (discardable_memory_limit_) {
    size_t limit = 0;
    if (bytes < discardable_memory_limit_)
      limit = discardable_memory_limit_ - bytes;

    PurgeLRUWithLockAcquiredUntilUsageIsWithin(limit);
  }

  // Check for overflow.
  if (std::numeric_limits<size_t>::max() - bytes < bytes_allocated_)
    return scoped_ptr<uint8, FreeDeleter>();

  scoped_ptr<uint8, FreeDeleter> memory(static_cast<uint8*>(malloc(bytes)));
  if (!memory)
    return scoped_ptr<uint8, FreeDeleter>();

  bytes_allocated_ += bytes;
  *purged = true;
  return memory.Pass();
}

void DiscardableMemoryProvider::Release(
    const DiscardableMemory* discardable,
    scoped_ptr<uint8, FreeDeleter> memory) {
  AutoLock lock(lock_);
  // NB: |allocations_| is an MRU cache, and use of |Get| here updates that
  // cache.
  AllocationMap::iterator it = allocations_.Get(discardable);
  CHECK(it != allocations_.end());

  DCHECK(!it->second.memory);
  it->second.memory = memory.release();

  EnforcePolicyWithLockAcquired();
}

void DiscardableMemoryProvider::PurgeAll() {
  AutoLock lock(lock_);
  PurgeLRUWithLockAcquiredUntilUsageIsWithin(0);
}

bool DiscardableMemoryProvider::IsRegisteredForTest(
    const DiscardableMemory* discardable) const {
  AutoLock lock(lock_);
  AllocationMap::const_iterator it = allocations_.Peek(discardable);
  return it != allocations_.end();
}

bool DiscardableMemoryProvider::CanBePurgedForTest(
    const DiscardableMemory* discardable) const {
  AutoLock lock(lock_);
  AllocationMap::const_iterator it = allocations_.Peek(discardable);
  return it != allocations_.end() && it->second.memory;
}

size_t DiscardableMemoryProvider::GetBytesAllocatedForTest() const {
  AutoLock lock(lock_);
  return bytes_allocated_;
}

void DiscardableMemoryProvider::Purge() {
  AutoLock lock(lock_);

  if (bytes_to_reclaim_under_moderate_pressure_ == 0)
    return;

  size_t limit = 0;
  if (bytes_to_reclaim_under_moderate_pressure_ < bytes_allocated_)
    limit = bytes_allocated_ - bytes_to_reclaim_under_moderate_pressure_;

  PurgeLRUWithLockAcquiredUntilUsageIsWithin(limit);
}

void DiscardableMemoryProvider::PurgeLRUWithLockAcquiredUntilUsageIsWithin(
    size_t limit) {
  TRACE_EVENT1(
      "base",
      "DiscardableMemoryProvider::PurgeLRUWithLockAcquiredUntilUsageIsWithin",
      "limit", limit);

  lock_.AssertAcquired();

  for (AllocationMap::reverse_iterator it = allocations_.rbegin();
       it != allocations_.rend();
       ++it) {
    if (bytes_allocated_ <= limit)
      break;
    if (!it->second.memory)
      continue;

    size_t bytes = it->second.bytes;
    DCHECK_LE(bytes, bytes_allocated_);
    bytes_allocated_ -= bytes;
    free(it->second.memory);
    it->second.memory = NULL;
  }
}

void DiscardableMemoryProvider::EnforcePolicyWithLockAcquired() {
  PurgeLRUWithLockAcquiredUntilUsageIsWithin(discardable_memory_limit_);
}

}  // namespace internal
}  // namespace base
