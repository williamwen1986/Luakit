// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/memory/discardable_memory_emulated.h"

#include "base/lazy_instance.h"
#include "base/memory/discardable_memory_provider.h"

namespace base {

namespace {

base::LazyInstance<internal::DiscardableMemoryProvider>::Leaky g_provider =
    LAZY_INSTANCE_INITIALIZER;

}  // namespace

namespace internal {

DiscardableMemoryEmulated::DiscardableMemoryEmulated(size_t size)
    : is_locked_(false) {
  g_provider.Pointer()->Register(this, size);
}

DiscardableMemoryEmulated::~DiscardableMemoryEmulated() {
  if (is_locked_)
    Unlock();
  g_provider.Pointer()->Unregister(this);
}

bool DiscardableMemoryEmulated::Initialize() {
  return Lock() == DISCARDABLE_MEMORY_LOCK_STATUS_PURGED;
}

DiscardableMemoryLockStatus DiscardableMemoryEmulated::Lock() {
  DCHECK(!is_locked_);

  bool purged = false;
  memory_ = g_provider.Pointer()->Acquire(this, &purged);
  if (!memory_)
    return DISCARDABLE_MEMORY_LOCK_STATUS_FAILED;

  is_locked_ = true;
  return purged ? DISCARDABLE_MEMORY_LOCK_STATUS_PURGED
                : DISCARDABLE_MEMORY_LOCK_STATUS_SUCCESS;
}

void DiscardableMemoryEmulated::Unlock() {
  DCHECK(is_locked_);
  g_provider.Pointer()->Release(this, memory_.Pass());
  is_locked_ = false;
}

void* DiscardableMemoryEmulated::Memory() const {
  DCHECK(memory_);
  return memory_.get();
}

// static
void DiscardableMemoryEmulated::PurgeForTesting() {
  g_provider.Pointer()->PurgeAll();
}

}  // namespace internal
}  // namespace base
