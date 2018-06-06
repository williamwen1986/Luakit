// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/memory/discardable_memory_android.h"

#include <sys/mman.h>
#include <unistd.h>

#include <limits>

#include "base/android/sys_utils.h"
#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/file_util.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/memory/discardable_memory_allocator_android.h"
#include "base/memory/discardable_memory_emulated.h"
#include "third_party/ashmem/ashmem.h"

namespace base {
namespace {

const char kAshmemAllocatorName[] = "DiscardableMemoryAllocator";

struct DiscardableMemoryAllocatorWrapper {
  DiscardableMemoryAllocatorWrapper()
      : allocator(kAshmemAllocatorName,
                  GetOptimalAshmemRegionSizeForAllocator()) {
  }

  internal::DiscardableMemoryAllocator allocator;

 private:
  // Returns 64 MBytes for a 512 MBytes device, 128 MBytes for 1024 MBytes...
  static size_t GetOptimalAshmemRegionSizeForAllocator() {
    // Note that this may do some I/O (without hitting the disk though) so it
    // should not be called on the critical path.
    return internal::AlignToNextPage(
        base::android::SysUtils::AmountOfPhysicalMemoryKB() * 1024 / 8);
  }
};

LazyInstance<DiscardableMemoryAllocatorWrapper>::Leaky g_context =
    LAZY_INSTANCE_INITIALIZER;

}  // namespace

namespace internal {

size_t AlignToNextPage(size_t size) {
  const size_t kPageSize = 4096;
  DCHECK_EQ(static_cast<int>(kPageSize), getpagesize());
  if (size > std::numeric_limits<size_t>::max() - kPageSize + 1)
    return 0;
  const size_t mask = ~(kPageSize - 1);
  return (size + kPageSize - 1) & mask;
}

bool CreateAshmemRegion(const char* name,
                        size_t size,
                        int* out_fd,
                        void** out_address) {
  int fd = ashmem_create_region(name, size);
  if (fd < 0) {
    DLOG(ERROR) << "ashmem_create_region() failed";
    return false;
  }
  file_util::ScopedFD fd_closer(&fd);

  const int err = ashmem_set_prot_region(fd, PROT_READ | PROT_WRITE);
  if (err < 0) {
    DLOG(ERROR) << "Error " << err << " when setting protection of ashmem";
    return false;
  }

  // There is a problem using MAP_PRIVATE here. As we are constantly calling
  // Lock() and Unlock(), data could get lost if they are not written to the
  // underlying file when Unlock() gets called.
  void* const address = mmap(
      NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (address == MAP_FAILED) {
    DPLOG(ERROR) << "Failed to map memory.";
    return false;
  }

  ignore_result(fd_closer.release());
  *out_fd = fd;
  *out_address = address;
  return true;
}

bool CloseAshmemRegion(int fd, size_t size, void* address) {
  if (munmap(address, size) == -1) {
    DPLOG(ERROR) << "Failed to unmap memory.";
    close(fd);
    return false;
  }
  return close(fd) == 0;
}

DiscardableMemoryLockStatus LockAshmemRegion(int fd,
                                             size_t off,
                                             size_t size,
                                             const void* address) {
  const int result = ashmem_pin_region(fd, off, size);
  DCHECK_EQ(0, mprotect(address, size, PROT_READ | PROT_WRITE));
  return result == ASHMEM_WAS_PURGED ? DISCARDABLE_MEMORY_LOCK_STATUS_PURGED
                                     : DISCARDABLE_MEMORY_LOCK_STATUS_SUCCESS;
}

bool UnlockAshmemRegion(int fd, size_t off, size_t size, const void* address) {
  const int failed = ashmem_unpin_region(fd, off, size);
  if (failed)
    DLOG(ERROR) << "Failed to unpin memory.";
  // This allows us to catch accesses to unlocked memory.
  DCHECK_EQ(0, mprotect(address, size, PROT_NONE));
  return !failed;
}

}  // namespace internal

// static
void DiscardableMemory::GetSupportedTypes(
    std::vector<DiscardableMemoryType>* types) {
  const DiscardableMemoryType supported_types[] = {
    DISCARDABLE_MEMORY_TYPE_ANDROID,
    DISCARDABLE_MEMORY_TYPE_EMULATED
  };
  types->assign(supported_types, supported_types + arraysize(supported_types));
}

// static
scoped_ptr<DiscardableMemory> DiscardableMemory::CreateLockedMemoryWithType(
    DiscardableMemoryType type, size_t size) {
  switch (type) {
    case DISCARDABLE_MEMORY_TYPE_NONE:
    case DISCARDABLE_MEMORY_TYPE_MAC:
      return scoped_ptr<DiscardableMemory>();
    case DISCARDABLE_MEMORY_TYPE_ANDROID: {
      return g_context.Pointer()->allocator.Allocate(size);
    }
    case DISCARDABLE_MEMORY_TYPE_EMULATED: {
      scoped_ptr<internal::DiscardableMemoryEmulated> memory(
          new internal::DiscardableMemoryEmulated(size));
      if (!memory->Initialize())
        return scoped_ptr<DiscardableMemory>();

      return memory.PassAs<DiscardableMemory>();
    }
  }

  NOTREACHED();
  return scoped_ptr<DiscardableMemory>();
}

// static
bool DiscardableMemory::PurgeForTestingSupported() {
  return false;
}

// static
void DiscardableMemory::PurgeForTesting() {
  NOTIMPLEMENTED();
}

}  // namespace base
