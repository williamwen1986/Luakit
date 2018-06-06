// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MEMORY_DISCARDABLE_MEMORY_PROVIDER_H_
#define BASE_MEMORY_DISCARDABLE_MEMORY_PROVIDER_H_

#include "base/base_export.h"
#include "base/containers/hash_tables.h"
#include "base/containers/mru_cache.h"
#include "base/memory/memory_pressure_listener.h"
#include "base/synchronization/lock.h"

namespace base {
class DiscardableMemory;
}  // namespace base

#if defined(COMPILER_GCC)
namespace BASE_HASH_NAMESPACE {
template <>
struct hash<const base::DiscardableMemory*> {
  size_t operator()(const base::DiscardableMemory* ptr) const {
    return hash<size_t>()(reinterpret_cast<size_t>(ptr));
  }
};
}  // namespace BASE_HASH_NAMESPACE
#endif  // COMPILER

namespace base {
namespace internal {

// The DiscardableMemoryProvider manages a collection of emulated
// DiscardableMemory instances. It is used on platforms that do not support
// discardable memory natively. It keeps track of all DiscardableMemory
// instances (in case they need to be purged), and the total amount of
// allocated memory (in case this forces a purge).
//
// When notified of memory pressure, the provider either purges the LRU
// memory -- if the pressure is moderate -- or all discardable memory
// if the pressure is critical.
//
// NB - this class is an implementation detail. It has been exposed for testing
// purposes. You should not need to use this class directly.
class BASE_EXPORT_PRIVATE DiscardableMemoryProvider {
 public:
  DiscardableMemoryProvider();
  ~DiscardableMemoryProvider();

  // The maximum number of bytes of discardable memory that may be allocated
  // before we force a purge. If this amount is zero, it is interpreted as
  // having no limit at all.
  void SetDiscardableMemoryLimit(size_t bytes);

  // Sets the amount of memory to reclaim when we're under moderate pressure.
  void SetBytesToReclaimUnderModeratePressure(size_t bytes);

  // Adds the given discardable memory to the provider's collection.
  void Register(const DiscardableMemory* discardable, size_t bytes);

  // Removes the given discardable memory from the provider's collection.
  void Unregister(const DiscardableMemory* discardable);

  // Returns NULL if an error occurred. Otherwise, returns the backing buffer
  // and sets |purged| to indicate whether or not the backing buffer has been
  // purged since last use.
  scoped_ptr<uint8, FreeDeleter> Acquire(
      const DiscardableMemory* discardable, bool* purged);

  // Release a previously acquired backing buffer. This gives the buffer back
  // to the provider where it can be purged if necessary.
  void Release(const DiscardableMemory* discardable,
               scoped_ptr<uint8, FreeDeleter> memory);

  // Purges all discardable memory.
  void PurgeAll();

  // Returns true if discardable memory has been added to the provider's
  // collection. This should only be used by tests.
  bool IsRegisteredForTest(const DiscardableMemory* discardable) const;

  // Returns true if discardable memory can be purged. This should only
  // be used by tests.
  bool CanBePurgedForTest(const DiscardableMemory* discardable) const;

  // Returns total amount of allocated discardable memory. This should only
  // be used by tests.
  size_t GetBytesAllocatedForTest() const;

 private:
  struct Allocation {
   explicit Allocation(size_t bytes)
       : bytes(bytes),
         memory(NULL) {
   }

    size_t bytes;
    uint8* memory;
  };
  typedef HashingMRUCache<const DiscardableMemory*, Allocation> AllocationMap;

  // This can be called as a hint that the system is under memory pressure.
  void NotifyMemoryPressure(
      MemoryPressureListener::MemoryPressureLevel pressure_level);

  // Purges |bytes_to_reclaim_under_moderate_pressure_| bytes of
  // discardable memory.
  void Purge();

  // Purges least recently used memory until usage is less or equal to |limit|.
  // Caller must acquire |lock_| prior to calling this function.
  void PurgeLRUWithLockAcquiredUntilUsageIsWithin(size_t limit);

  // Ensures that we don't allocate beyond our memory limit.
  // Caller must acquire |lock_| prior to calling this function.
  void EnforcePolicyWithLockAcquired();

  // Needs to be held when accessing members.
  mutable Lock lock_;

  // A MRU cache of all allocated bits of discardable memory. Used for purging.
  AllocationMap allocations_;

  // The total amount of allocated discardable memory.
  size_t bytes_allocated_;

  // The maximum number of bytes of discardable memory that may be allocated
  // before we assume moderate memory pressure.
  size_t discardable_memory_limit_;

  // Under moderate memory pressure, we will purge this amount of memory.
  size_t bytes_to_reclaim_under_moderate_pressure_;

  // Allows us to be respond when the system reports that it is under memory
  // pressure.
  MemoryPressureListener memory_pressure_listener_;

  DISALLOW_COPY_AND_ASSIGN(DiscardableMemoryProvider);
};

}  // namespace internal
}  // namespace base

#endif  // BASE_MEMORY_DISCARDABLE_MEMORY_PROVIDER_H_
