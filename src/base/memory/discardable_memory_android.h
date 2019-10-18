// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Please use discardable_memory.h since this is just an internal file providing
// utility functions used both by discardable_memory_android.cc and
// discardable_memory_allocator_android.cc.

#ifndef BASE_MEMORY_DISCARDABLE_MEMORY_ANDROID_H_
#define BASE_MEMORY_DISCARDABLE_MEMORY_ANDROID_H_

#include "base/basictypes.h"
#include "base/memory/discardable_memory.h"

namespace base {
namespace internal {

// Returns 0 if the provided size is too high to be aligned.
size_t AlignToNextPage(size_t size);

bool CreateAshmemRegion(const char* name, size_t size, int* fd, void** address);

bool CloseAshmemRegion(int fd, size_t size, void* address);

DiscardableMemoryLockStatus LockAshmemRegion(int fd,
                                             size_t offset,
                                             size_t size,
                                             const void* address);

bool UnlockAshmemRegion(int fd,
                        size_t offset,
                        size_t size,
                        const void* address);

}  // namespace internal
}  // namespace base

#endif  // BASE_MEMORY_DISCARDABLE_MEMORY_ANDROID_H_
