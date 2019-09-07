// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MEMORY_DISCARDABLE_MEMORY_ALLOCATOR_H_
#define BASE_MEMORY_DISCARDABLE_MEMORY_ALLOCATOR_H_

#include <string>

#include "base/base_export.h"
#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/scoped_vector.h"
#include "base/synchronization/lock.h"
#include "base/threading/thread_checker.h"

namespace base {

class DiscardableMemory;

namespace internal {

// On Android ashmem is used to implement discardable memory. It is backed by a
// file (descriptor) thus is a limited resource. This allocator minimizes the
// problem by allocating large ashmem regions internally and returning smaller
// chunks to the client.
// Allocated chunks are systematically aligned on a page boundary therefore this
// allocator should not be used for small allocations.
//
// Threading: The allocator must be deleted on the thread it was constructed on
// although its Allocate() method can be invoked on any thread. See
// discardable_memory.h for DiscardableMemory's threading guarantees.
class BASE_EXPORT_PRIVATE DiscardableMemoryAllocator {
 public:
  // Note that |name| is only used for debugging/measurement purposes.
  // |ashmem_region_size| is the size that will be used to create the underlying
  // ashmem regions and is expected to be greater or equal than 32 MBytes.
  DiscardableMemoryAllocator(const std::string& name,
                             size_t ashmem_region_size);

  ~DiscardableMemoryAllocator();

  // Note that the allocator must outlive the returned DiscardableMemory
  // instance.
  scoped_ptr<DiscardableMemory> Allocate(size_t size);

 private:
  class AshmemRegion;
  class DiscardableAshmemChunk;

  void DeleteAshmemRegion_Locked(AshmemRegion* region);

  base::ThreadChecker thread_checker_;
  const std::string name_;
  const size_t ashmem_region_size_;
  base::Lock lock_;
  ScopedVector<AshmemRegion> ashmem_regions_;

  DISALLOW_COPY_AND_ASSIGN(DiscardableMemoryAllocator);
};

}  // namespace internal
}  // namespace base

#endif  // BASE_MEMORY_DISCARDABLE_MEMORY_ALLOCATOR_H_
