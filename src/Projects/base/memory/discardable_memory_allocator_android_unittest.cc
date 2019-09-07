// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/memory/discardable_memory_allocator_android.h"

#include <sys/types.h>
#include <unistd.h>

#include "base/memory/discardable_memory.h"
#include "base/memory/scoped_ptr.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/stringprintf.h"
#include "build/build_config.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace base {
namespace internal {

const char kAllocatorName[] = "allocator-for-testing";

const size_t kAshmemRegionSizeForTesting = 32 * 1024 * 1024;
const size_t kPageSize = 4096;

class DiscardableMemoryAllocatorTest : public testing::Test {
 protected:
  DiscardableMemoryAllocatorTest()
      : allocator_(kAllocatorName, kAshmemRegionSizeForTesting) {
  }

  DiscardableMemoryAllocator allocator_;
};

void WriteToDiscardableMemory(DiscardableMemory* memory, size_t size) {
  // Write to the first and the last pages only to avoid paging in up to 64
  // MBytes.
  static_cast<char*>(memory->Memory())[0] = 'a';
  static_cast<char*>(memory->Memory())[size - 1] = 'a';
}

TEST_F(DiscardableMemoryAllocatorTest, Basic) {
  const size_t size = 128;
  scoped_ptr<DiscardableMemory> memory(allocator_.Allocate(size));
  ASSERT_TRUE(memory);
  WriteToDiscardableMemory(memory.get(), size);
}

TEST_F(DiscardableMemoryAllocatorTest, ZeroAllocationIsNotSupported) {
  scoped_ptr<DiscardableMemory> memory(allocator_.Allocate(0));
  ASSERT_FALSE(memory);
}

TEST_F(DiscardableMemoryAllocatorTest, TooLargeAllocationFails) {
  const size_t max_allowed_allocation_size =
      std::numeric_limits<size_t>::max() - kPageSize + 1;
  scoped_ptr<DiscardableMemory> memory(
      allocator_.Allocate(max_allowed_allocation_size + 1));
  // Page-alignment would have caused an overflow resulting in a small
  // allocation if the input size wasn't checked correctly.
  ASSERT_FALSE(memory);
}

TEST_F(DiscardableMemoryAllocatorTest,
       AshmemRegionsAreNotSmallerThanRequestedSize) {
  const size_t size = std::numeric_limits<size_t>::max() - kPageSize + 1;
  // The creation of the underlying ashmem region is expected to fail since
  // there should not be enough room in the address space. When ashmem creation
  // fails, the allocator repetitively retries by dividing the size by 2. This
  // size should not be smaller than the size the user requested so the
  // allocation here should just fail (and not succeed with the minimum ashmem
  // region size).
  scoped_ptr<DiscardableMemory> memory(allocator_.Allocate(size));
  ASSERT_FALSE(memory);
}

TEST_F(DiscardableMemoryAllocatorTest, LargeAllocation) {
  // Note that large allocations should just use DiscardableMemoryAndroidSimple
  // instead.
  const size_t size = 64 * 1024 * 1024;
  scoped_ptr<DiscardableMemory> memory(allocator_.Allocate(size));
  ASSERT_TRUE(memory);
  WriteToDiscardableMemory(memory.get(), size);
}

TEST_F(DiscardableMemoryAllocatorTest, ChunksArePageAligned) {
  scoped_ptr<DiscardableMemory> memory(allocator_.Allocate(kPageSize));
  ASSERT_TRUE(memory);
  EXPECT_EQ(0U, reinterpret_cast<uint64_t>(memory->Memory()) % kPageSize);
  WriteToDiscardableMemory(memory.get(), kPageSize);
}

TEST_F(DiscardableMemoryAllocatorTest, AllocateFreeAllocate) {
  scoped_ptr<DiscardableMemory> memory(allocator_.Allocate(kPageSize));
  // Extra allocation that prevents the region from being deleted when |memory|
  // gets deleted.
  scoped_ptr<DiscardableMemory> memory_lock(allocator_.Allocate(kPageSize));
  ASSERT_TRUE(memory);
  void* const address = memory->Memory();
  memory->Unlock();  // Tests that the reused chunk is being locked correctly.
  memory.reset();
  memory = allocator_.Allocate(kPageSize);
  ASSERT_TRUE(memory);
  // The previously freed chunk should be reused.
  EXPECT_EQ(address, memory->Memory());
  WriteToDiscardableMemory(memory.get(), kPageSize);
}

TEST_F(DiscardableMemoryAllocatorTest, FreeingWholeAshmemRegionClosesAshmem) {
  scoped_ptr<DiscardableMemory> memory(allocator_.Allocate(kPageSize));
  ASSERT_TRUE(memory);
  const int kMagic = 0xdeadbeef;
  *static_cast<int*>(memory->Memory()) = kMagic;
  memory.reset();
  // The previous ashmem region should have been closed thus it should not be
  // reused.
  memory = allocator_.Allocate(kPageSize);
  ASSERT_TRUE(memory);
  EXPECT_NE(kMagic, *static_cast<const int*>(memory->Memory()));
}

TEST_F(DiscardableMemoryAllocatorTest, AllocateUsesBestFitAlgorithm) {
  scoped_ptr<DiscardableMemory> memory1(allocator_.Allocate(3 * kPageSize));
  ASSERT_TRUE(memory1);
  scoped_ptr<DiscardableMemory> memory2(allocator_.Allocate(2 * kPageSize));
  ASSERT_TRUE(memory2);
  scoped_ptr<DiscardableMemory> memory3(allocator_.Allocate(1 * kPageSize));
  ASSERT_TRUE(memory3);
  void* const address_3 = memory3->Memory();
  memory1.reset();
  // Don't free |memory2| to avoid merging the 3 blocks together.
  memory3.reset();
  memory1 = allocator_.Allocate(1 * kPageSize);
  ASSERT_TRUE(memory1);
  // The chunk whose size is closest to the requested size should be reused.
  EXPECT_EQ(address_3, memory1->Memory());
  WriteToDiscardableMemory(memory1.get(), kPageSize);
}

TEST_F(DiscardableMemoryAllocatorTest, MergeFreeChunks) {
  scoped_ptr<DiscardableMemory> memory1(allocator_.Allocate(kPageSize));
  ASSERT_TRUE(memory1);
  scoped_ptr<DiscardableMemory> memory2(allocator_.Allocate(kPageSize));
  ASSERT_TRUE(memory2);
  scoped_ptr<DiscardableMemory> memory3(allocator_.Allocate(kPageSize));
  ASSERT_TRUE(memory3);
  scoped_ptr<DiscardableMemory> memory4(allocator_.Allocate(kPageSize));
  ASSERT_TRUE(memory4);
  void* const memory1_address = memory1->Memory();
  memory1.reset();
  memory3.reset();
  // Freeing |memory2| (located between memory1 and memory3) should merge the
  // three free blocks together.
  memory2.reset();
  memory1 = allocator_.Allocate(3 * kPageSize);
  EXPECT_EQ(memory1_address, memory1->Memory());
}

TEST_F(DiscardableMemoryAllocatorTest, MergeFreeChunksAdvanced) {
  scoped_ptr<DiscardableMemory> memory1(allocator_.Allocate(4 * kPageSize));
  ASSERT_TRUE(memory1);
  scoped_ptr<DiscardableMemory> memory2(allocator_.Allocate(4 * kPageSize));
  ASSERT_TRUE(memory2);
  void* const memory1_address = memory1->Memory();
  memory1.reset();
  memory1 = allocator_.Allocate(2 * kPageSize);
  memory2.reset();
  // At this point, the region should be in this state:
  // 8 KBytes (used), 24 KBytes (free).
  memory2 = allocator_.Allocate(6 * kPageSize);
  EXPECT_EQ(
      static_cast<const char*>(memory2->Memory()),
      static_cast<const char*>(memory1_address) + 2 * kPageSize);
}

TEST_F(DiscardableMemoryAllocatorTest, MergeFreeChunksAdvanced2) {
  scoped_ptr<DiscardableMemory> memory1(allocator_.Allocate(4 * kPageSize));
  ASSERT_TRUE(memory1);
  scoped_ptr<DiscardableMemory> memory2(allocator_.Allocate(4 * kPageSize));
  ASSERT_TRUE(memory2);
  void* const memory1_address = memory1->Memory();
  memory1.reset();
  memory1 = allocator_.Allocate(2 * kPageSize);
  scoped_ptr<DiscardableMemory> memory3(allocator_.Allocate(2 * kPageSize));
  // At this point, the region should be in this state:
  // 8 KBytes (used), 8 KBytes (used), 16 KBytes (used).
  memory3.reset();
  memory2.reset();
  // At this point, the region should be in this state:
  // 8 KBytes (used), 24 KBytes (free).
  memory2 = allocator_.Allocate(6 * kPageSize);
  EXPECT_EQ(
      static_cast<const char*>(memory2->Memory()),
      static_cast<const char*>(memory1_address) + 2 * kPageSize);
}

TEST_F(DiscardableMemoryAllocatorTest, MergeFreeChunksAndDeleteAshmemRegion) {
  scoped_ptr<DiscardableMemory> memory1(allocator_.Allocate(4 * kPageSize));
  ASSERT_TRUE(memory1);
  scoped_ptr<DiscardableMemory> memory2(allocator_.Allocate(4 * kPageSize));
  ASSERT_TRUE(memory2);
  memory1.reset();
  memory1 = allocator_.Allocate(2 * kPageSize);
  scoped_ptr<DiscardableMemory> memory3(allocator_.Allocate(2 * kPageSize));
  // At this point, the region should be in this state:
  // 8 KBytes (used), 8 KBytes (used), 16 KBytes (used).
  memory1.reset();
  memory3.reset();
  // At this point, the region should be in this state:
  // 8 KBytes (free), 8 KBytes (used), 8 KBytes (free).
  const int kMagic = 0xdeadbeef;
  *static_cast<int*>(memory2->Memory()) = kMagic;
  memory2.reset();
  // The whole region should have been deleted.
  memory2 = allocator_.Allocate(2 * kPageSize);
  EXPECT_NE(kMagic, *static_cast<int*>(memory2->Memory()));
}

TEST_F(DiscardableMemoryAllocatorTest,
     TooLargeFreeChunksDontCauseTooMuchFragmentationWhenRecycled) {
  // Keep |memory_1| below allocated so that the ashmem region doesn't get
  // closed when |memory_2| is deleted.
  scoped_ptr<DiscardableMemory> memory_1(allocator_.Allocate(64 * 1024));
  ASSERT_TRUE(memory_1);
  scoped_ptr<DiscardableMemory> memory_2(allocator_.Allocate(32 * 1024));
  ASSERT_TRUE(memory_2);
  void* const address = memory_2->Memory();
  memory_2.reset();
  const size_t size = 16 * 1024;
  memory_2 = allocator_.Allocate(size);
  ASSERT_TRUE(memory_2);
  EXPECT_EQ(address, memory_2->Memory());
  WriteToDiscardableMemory(memory_2.get(), size);
  scoped_ptr<DiscardableMemory> memory_3(allocator_.Allocate(size));
  // The unused tail (16 KBytes large) of the previously freed chunk should be
  // reused.
  EXPECT_EQ(static_cast<char*>(address) + size, memory_3->Memory());
  WriteToDiscardableMemory(memory_3.get(), size);
}

TEST_F(DiscardableMemoryAllocatorTest, UseMultipleAshmemRegions) {
  // Leave one page untouched at the end of the ashmem region.
  const size_t size = kAshmemRegionSizeForTesting - kPageSize;
  scoped_ptr<DiscardableMemory> memory1(allocator_.Allocate(size));
  ASSERT_TRUE(memory1);
  WriteToDiscardableMemory(memory1.get(), size);

  scoped_ptr<DiscardableMemory> memory2(
      allocator_.Allocate(kAshmemRegionSizeForTesting));
  ASSERT_TRUE(memory2);
  WriteToDiscardableMemory(memory2.get(), kAshmemRegionSizeForTesting);
  // The last page of the first ashmem region should be used for this
  // allocation.
  scoped_ptr<DiscardableMemory> memory3(allocator_.Allocate(kPageSize));
  ASSERT_TRUE(memory3);
  WriteToDiscardableMemory(memory3.get(), kPageSize);
  EXPECT_EQ(memory3->Memory(), static_cast<char*>(memory1->Memory()) + size);
}

}  // namespace internal
}  // namespace base
