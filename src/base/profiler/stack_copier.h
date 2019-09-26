// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_PROFILER_STACK_COPIER_H_
#define BASE_PROFILER_STACK_COPIER_H_

#include <stdint.h>

#include "base/base_export.h"
#include "base/profiler/register_context.h"

namespace base {

class StackBuffer;
class ProfileBuilder;

// StackCopier causes a thread to be suspended, copies its stack, and resumes
// the thread's execution. It's intended to provide an abstraction over stack
// copying techniques where the thread suspension is performed directly by the
// profiler thread (Windows and Mac platforms) vs. where the thread suspension
// is performed by the OS through signals (Android).
class BASE_EXPORT StackCopier {
 public:
  virtual ~StackCopier();

  // Copies the thread's register context into |thread_context|, the stack into
  // |stack_buffer|, and the top of stack address into |stack_top|. Records
  // metadata while the thread is suspended via |profile_builder|. Returns true
  // if successful.
  virtual bool CopyStack(StackBuffer* stack_buffer,
                         uintptr_t* stack_top,
                         ProfileBuilder* profile_builder,
                         RegisterContext* thread_context) = 0;

 protected:
  // If the value at |pointer| points to the original stack, rewrite it to point
  // to the corresponding location in the copied stack.
  //
  // NO HEAP ALLOCATIONS.
  static uintptr_t RewritePointerIfInOriginalStack(
      const uint8_t* original_stack_bottom,
      const uintptr_t* original_stack_top,
      const uint8_t* stack_copy_bottom,
      uintptr_t pointer);

  // Copies the stack to a buffer while rewriting possible pointers to locations
  // within the stack to point to the corresponding locations in the copy. This
  // is necessary to handle stack frames with dynamic stack allocation, where a
  // pointer to the beginning of the dynamic allocation area is stored on the
  // stack and/or in a non-volatile register.
  //
  // Eager rewriting of anything that looks like a pointer to the stack, as done
  // in this function, does not adversely affect the stack unwinding. The only
  // other values on the stack the unwinding depends on are return addresses,
  // which should not point within the stack memory. The rewriting is guaranteed
  // to catch all pointers because the stacks are guaranteed by the ABI to be
  // sizeof(uintptr_t*) aligned.
  //
  // |original_stack_bottom| and |original_stack_top| are different pointer
  // types due on their differing guaranteed alignments -- the bottom may only
  // be 1-byte aligned while the top is aligned to double the pointer width.
  //
  // Returns a pointer to the bottom address in the copied stack. This value
  // matches the alignment of |original_stack_bottom| to ensure that the stack
  // contents have the same alignment as in the original stack. As a result the
  // value will be different than |stack_buffer_bottom| if
  // |original_stack_bottom| is not aligned to double the pointer width.
  //
  // NO HEAP ALLOCATIONS.
  static const uint8_t* CopyStackContentsAndRewritePointers(
      const uint8_t* original_stack_bottom,
      const uintptr_t* original_stack_top,
      int platform_stack_alignment,
      uintptr_t* stack_buffer_bottom);
};

}  // namespace base

#endif  // BASE_PROFILER_STACK_COPIER_H_
