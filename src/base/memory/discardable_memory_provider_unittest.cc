// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/memory/discardable_memory_provider.h"

#include "base/bind.h"
#include "base/memory/discardable_memory.h"
#include "base/run_loop.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace base {

class DiscardableMemoryProviderTestBase {
 public:
  class TestDiscardableMemory : public DiscardableMemory {
   public:
    TestDiscardableMemory(
        internal::DiscardableMemoryProvider* provider, size_t size)
        : provider_(provider),
          is_locked_(false) {
      provider_->Register(this, size);
    }

    virtual ~TestDiscardableMemory() {
      if (is_locked_)
        Unlock();
      provider_->Unregister(this);
    }

    // Overridden from DiscardableMemory:
    virtual DiscardableMemoryLockStatus Lock() OVERRIDE {
      DCHECK(!is_locked_);

      bool purged = false;
      memory_ = provider_->Acquire(this, &purged);
      if (!memory_)
        return DISCARDABLE_MEMORY_LOCK_STATUS_FAILED;

      is_locked_ = true;
      return purged ? DISCARDABLE_MEMORY_LOCK_STATUS_PURGED
                    : DISCARDABLE_MEMORY_LOCK_STATUS_SUCCESS;
    }
    virtual void Unlock() OVERRIDE {
      DCHECK(is_locked_);
      provider_->Release(this, memory_.Pass());
      is_locked_ = false;
    }
    virtual void* Memory() const OVERRIDE {
      DCHECK(memory_);
      return memory_.get();
    }

   private:
    internal::DiscardableMemoryProvider* provider_;
    scoped_ptr<uint8, FreeDeleter> memory_;
    bool is_locked_;

    DISALLOW_COPY_AND_ASSIGN(TestDiscardableMemory);
  };

  DiscardableMemoryProviderTestBase()
      : message_loop_(MessageLoop::TYPE_IO),
        provider_(new internal::DiscardableMemoryProvider) {
  }

 protected:
  bool IsRegistered(const DiscardableMemory* discardable) {
    return provider_->IsRegisteredForTest(discardable);
  }

  bool CanBePurged(const DiscardableMemory* discardable) {
    return provider_->CanBePurgedForTest(discardable);
  }

  size_t BytesAllocated() const {
    return provider_->GetBytesAllocatedForTest();
  }

  void* Memory(const DiscardableMemory* discardable) const {
    return discardable->Memory();
  }

  void SetDiscardableMemoryLimit(size_t bytes) {
    provider_->SetDiscardableMemoryLimit(bytes);
  }

  void SetBytesToReclaimUnderModeratePressure(size_t bytes) {
    provider_->SetBytesToReclaimUnderModeratePressure(bytes);
  }

  scoped_ptr<DiscardableMemory> CreateLockedMemory(size_t size) {
    scoped_ptr<TestDiscardableMemory> memory(
        new TestDiscardableMemory(provider_.get(), size));
    if (memory->Lock() != DISCARDABLE_MEMORY_LOCK_STATUS_PURGED)
      return scoped_ptr<DiscardableMemory>();
    return memory.PassAs<DiscardableMemory>();
  }

 private:
  MessageLoop message_loop_;
  scoped_ptr<internal::DiscardableMemoryProvider> provider_;
};

class DiscardableMemoryProviderTest
    : public DiscardableMemoryProviderTestBase,
      public testing::Test {
 public:
  DiscardableMemoryProviderTest() {}
};

TEST_F(DiscardableMemoryProviderTest, CreateLockedMemory) {
  size_t size = 1024;
  const scoped_ptr<DiscardableMemory> discardable(CreateLockedMemory(size));
  EXPECT_TRUE(IsRegistered(discardable.get()));
  EXPECT_NE(static_cast<void*>(NULL), Memory(discardable.get()));
  EXPECT_EQ(1024u, BytesAllocated());
  EXPECT_FALSE(CanBePurged(discardable.get()));
}

TEST_F(DiscardableMemoryProviderTest, CreateLockedMemoryZeroSize) {
  size_t size = 0;
  const scoped_ptr<DiscardableMemory> discardable(CreateLockedMemory(size));
  EXPECT_FALSE(discardable);
  EXPECT_FALSE(IsRegistered(discardable.get()));
  EXPECT_EQ(0u, BytesAllocated());
}

TEST_F(DiscardableMemoryProviderTest, LockAfterUnlock) {
  size_t size = 1024;
  const scoped_ptr<DiscardableMemory> discardable(CreateLockedMemory(size));
  EXPECT_TRUE(IsRegistered(discardable.get()));
  EXPECT_NE(static_cast<void*>(NULL), Memory(discardable.get()));
  EXPECT_EQ(1024u, BytesAllocated());
  EXPECT_FALSE(CanBePurged(discardable.get()));

  // Now unlock so we can lock later.
  discardable->Unlock();
  EXPECT_TRUE(CanBePurged(discardable.get()));

  EXPECT_EQ(DISCARDABLE_MEMORY_LOCK_STATUS_SUCCESS, discardable->Lock());
  EXPECT_FALSE(CanBePurged(discardable.get()));
}

TEST_F(DiscardableMemoryProviderTest, LockAfterPurge) {
  size_t size = 1024;
  const scoped_ptr<DiscardableMemory> discardable(CreateLockedMemory(size));
  EXPECT_TRUE(IsRegistered(discardable.get()));
  EXPECT_NE(static_cast<void*>(NULL), Memory(discardable.get()));
  EXPECT_EQ(1024u, BytesAllocated());
  EXPECT_FALSE(CanBePurged(discardable.get()));

  // Now unlock so we can lock later.
  discardable->Unlock();
  EXPECT_TRUE(CanBePurged(discardable.get()));

  // Force the system to purge.
  MemoryPressureListener::NotifyMemoryPressure(
      MemoryPressureListener::MEMORY_PRESSURE_CRITICAL);

  // Required because ObserverListThreadSafe notifies via PostTask.
  RunLoop().RunUntilIdle();

  EXPECT_EQ(DISCARDABLE_MEMORY_LOCK_STATUS_PURGED, discardable->Lock());
  EXPECT_FALSE(CanBePurged(discardable.get()));
}

TEST_F(DiscardableMemoryProviderTest, LockAfterPurgeAndCannotReallocate) {
  size_t size = 1024;
  const scoped_ptr<DiscardableMemory> discardable(CreateLockedMemory(size));
  EXPECT_TRUE(IsRegistered(discardable.get()));
  EXPECT_NE(static_cast<void*>(NULL), Memory(discardable.get()));
  EXPECT_EQ(1024u, BytesAllocated());
  EXPECT_FALSE(CanBePurged(discardable.get()));

  // Now unlock so we can lock later.
  discardable->Unlock();
  EXPECT_TRUE(CanBePurged(discardable.get()));

  // Set max allowed allocation to 1 byte. This will make cause the memory
  // to be purged.
  SetDiscardableMemoryLimit(1);

  EXPECT_EQ(DISCARDABLE_MEMORY_LOCK_STATUS_PURGED, discardable->Lock());
  EXPECT_FALSE(CanBePurged(discardable.get()));
}

TEST_F(DiscardableMemoryProviderTest, Overflow) {
  {
    size_t size = 1024;
    const scoped_ptr<DiscardableMemory> discardable(CreateLockedMemory(size));
    EXPECT_TRUE(IsRegistered(discardable.get()));
    EXPECT_NE(static_cast<void*>(NULL), Memory(discardable.get()));
    EXPECT_EQ(1024u, BytesAllocated());

    size_t massive_size = std::numeric_limits<size_t>::max();
    const scoped_ptr<DiscardableMemory> massive_discardable(
        CreateLockedMemory(massive_size));
    EXPECT_FALSE(massive_discardable);
    EXPECT_EQ(1024u, BytesAllocated());
  }
  EXPECT_EQ(0u, BytesAllocated());
}

class PermutationTestData {
 public:
  PermutationTestData(unsigned d0, unsigned d1, unsigned d2) {
    ordering_[0] = d0;
    ordering_[1] = d1;
    ordering_[2] = d2;
  }

  const unsigned* ordering() const { return ordering_; }

 private:
  unsigned ordering_[3];
};

class DiscardableMemoryProviderPermutationTest
    : public DiscardableMemoryProviderTestBase,
      public testing::TestWithParam<PermutationTestData> {
 public:
  DiscardableMemoryProviderPermutationTest() {}

 protected:
  // Use discardable memory in order specified by ordering parameter.
  void CreateAndUseDiscardableMemory() {
    for (int i = 0; i < 3; ++i) {
      discardables_[i] = CreateLockedMemory(1024);
      EXPECT_TRUE(discardables_[i]);
      EXPECT_NE(static_cast<void*>(NULL), Memory(discardables_[i].get()));
      discardables_[i]->Unlock();
    }
    for (int i = 0; i < 3; ++i) {
      int index = GetParam().ordering()[i];
      EXPECT_NE(DISCARDABLE_MEMORY_LOCK_STATUS_FAILED,
                discardables_[index]->Lock());
      // Leave i == 0 locked.
      if (i > 0)
        discardables_[index]->Unlock();
    }
  }

  DiscardableMemory* discardable(unsigned position) {
    return discardables_[GetParam().ordering()[position]].get();
  }

 private:
  scoped_ptr<DiscardableMemory> discardables_[3];
};

// Verify that memory was discarded in the correct order after applying
// memory pressure.
TEST_P(DiscardableMemoryProviderPermutationTest, LRUDiscardedModeratePressure) {
  CreateAndUseDiscardableMemory();

  SetBytesToReclaimUnderModeratePressure(1024);
  MemoryPressureListener::NotifyMemoryPressure(
      MemoryPressureListener::MEMORY_PRESSURE_MODERATE);
  RunLoop().RunUntilIdle();

  EXPECT_NE(DISCARDABLE_MEMORY_LOCK_STATUS_FAILED, discardable(2)->Lock());
  EXPECT_NE(DISCARDABLE_MEMORY_LOCK_STATUS_SUCCESS, discardable(1)->Lock());
  // 0 should still be locked.
  EXPECT_NE(static_cast<void*>(NULL), Memory(discardable(0)));
}

// Verify that memory was discarded in the correct order after changing
// memory limit.
TEST_P(DiscardableMemoryProviderPermutationTest, LRUDiscardedExceedLimit) {
  CreateAndUseDiscardableMemory();

  SetBytesToReclaimUnderModeratePressure(1024);
  SetDiscardableMemoryLimit(2048);

  EXPECT_NE(DISCARDABLE_MEMORY_LOCK_STATUS_FAILED, discardable(2)->Lock());
  EXPECT_NE(DISCARDABLE_MEMORY_LOCK_STATUS_SUCCESS, discardable(1)->Lock());
  // 0 should still be locked.
  EXPECT_NE(static_cast<void*>(NULL), Memory(discardable(0)));
}

// Verify that no more memory than necessary was discarded after changing
// memory limit.
TEST_P(DiscardableMemoryProviderPermutationTest, LRUDiscardedAmount) {
  SetBytesToReclaimUnderModeratePressure(2048);
  SetDiscardableMemoryLimit(4096);

  CreateAndUseDiscardableMemory();

  SetDiscardableMemoryLimit(2048);

  EXPECT_EQ(DISCARDABLE_MEMORY_LOCK_STATUS_SUCCESS, discardable(2)->Lock());
  EXPECT_EQ(DISCARDABLE_MEMORY_LOCK_STATUS_PURGED, discardable(1)->Lock());
  // 0 should still be locked.
  EXPECT_NE(static_cast<void*>(NULL), Memory(discardable(0)));
}

TEST_P(DiscardableMemoryProviderPermutationTest,
       CriticalPressureFreesAllUnlocked) {
  CreateAndUseDiscardableMemory();

  MemoryPressureListener::NotifyMemoryPressure(
      MemoryPressureListener::MEMORY_PRESSURE_CRITICAL);
  RunLoop().RunUntilIdle();

  for (int i = 0; i < 3; ++i) {
    if (i == 0)
      EXPECT_NE(static_cast<void*>(NULL), Memory(discardable(i)));
    else
      EXPECT_EQ(DISCARDABLE_MEMORY_LOCK_STATUS_PURGED, discardable(i)->Lock());
  }
}

INSTANTIATE_TEST_CASE_P(DiscardableMemoryProviderPermutationTests,
                        DiscardableMemoryProviderPermutationTest,
                        ::testing::Values(PermutationTestData(0, 1, 2),
                                          PermutationTestData(0, 2, 1),
                                          PermutationTestData(1, 0, 2),
                                          PermutationTestData(1, 2, 0),
                                          PermutationTestData(2, 0, 1),
                                          PermutationTestData(2, 1, 0)));

TEST_F(DiscardableMemoryProviderTest, NormalDestruction) {
  {
    size_t size = 1024;
    const scoped_ptr<DiscardableMemory> discardable(CreateLockedMemory(size));
    EXPECT_TRUE(IsRegistered(discardable.get()));
    EXPECT_EQ(1024u, BytesAllocated());
  }
  EXPECT_EQ(0u, BytesAllocated());
}

TEST_F(DiscardableMemoryProviderTest, DestructionWhileLocked) {
  {
    size_t size = 1024;
    const scoped_ptr<DiscardableMemory> discardable(CreateLockedMemory(size));
    EXPECT_TRUE(IsRegistered(discardable.get()));
    EXPECT_NE(static_cast<void*>(NULL), Memory(discardable.get()));
    EXPECT_EQ(1024u, BytesAllocated());
    EXPECT_FALSE(CanBePurged(discardable.get()));
  }
  // Should have ignored the "locked" status and freed the discardable memory.
  EXPECT_EQ(0u, BytesAllocated());
}

#if !defined(NDEBUG) && !defined(OS_ANDROID) && !defined(OS_IOS)
// Death tests are not supported with Android APKs.
TEST_F(DiscardableMemoryProviderTest, UnlockedMemoryAccessCrashesInDebugMode) {
  size_t size = 1024;
  const scoped_ptr<DiscardableMemory> discardable(CreateLockedMemory(size));
  EXPECT_TRUE(IsRegistered(discardable.get()));
  EXPECT_NE(static_cast<void*>(NULL), Memory(discardable.get()));
  EXPECT_EQ(1024u, BytesAllocated());
  EXPECT_FALSE(CanBePurged(discardable.get()));
  discardable->Unlock();
  EXPECT_TRUE(CanBePurged(discardable.get()));
  // We *must* die if we are asked to vend a pointer to unlocked memory.
  EXPECT_DEATH(discardable->Memory(), ".*Check failed.*");
}
#endif

class ThreadedDiscardableMemoryProviderTest
    : public DiscardableMemoryProviderTest {
 public:
  ThreadedDiscardableMemoryProviderTest()
      : memory_usage_thread_("memory_usage_thread"),
        thread_sync_(true, false) {
  }

  virtual void SetUp() OVERRIDE {
    memory_usage_thread_.Start();
  }

  virtual void TearDown() OVERRIDE {
    memory_usage_thread_.Stop();
  }

  void UseMemoryHelper() {
    size_t size = 1024;
    const scoped_ptr<DiscardableMemory> discardable(CreateLockedMemory(size));
    EXPECT_TRUE(IsRegistered(discardable.get()));
    EXPECT_NE(static_cast<void*>(NULL), Memory(discardable.get()));
    discardable->Unlock();
  }

  void SignalHelper() {
    thread_sync_.Signal();
  }

  Thread memory_usage_thread_;
  WaitableEvent thread_sync_;
};

TEST_F(ThreadedDiscardableMemoryProviderTest, UseMemoryOnThread) {
  memory_usage_thread_.message_loop()->PostTask(
      FROM_HERE,
      Bind(&ThreadedDiscardableMemoryProviderTest::UseMemoryHelper,
           Unretained(this)));
  memory_usage_thread_.message_loop()->PostTask(
      FROM_HERE,
      Bind(&ThreadedDiscardableMemoryProviderTest::SignalHelper,
           Unretained(this)));
  thread_sync_.Wait();
}

}  // namespace base
