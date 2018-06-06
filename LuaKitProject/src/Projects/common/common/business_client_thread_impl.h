// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_BROWSER_THREAD_IMPL_H_
#define CONTENT_BROWSER_BROWSER_THREAD_IMPL_H_
#pragma once

#include "base/threading/thread.h"
#include "common/business_client_thread.h"

class BusinessThreadImpl
    : public BusinessThread, public base::Thread {
 public:
  // Construct a BusinessThreadImpl with the supplied identifier.  It is an error
  // to construct a BusinessThreadImpl that already exists.
  explicit BusinessThreadImpl(BusinessThreadID identifier, const char * thread_name);

  // Special constructor for the main (UI) thread and unittests. We use a dummy
  // thread here since the main thread already exists.
  BusinessThreadImpl(BusinessThreadID identifier, base::MessageLoop* message_loop);
  virtual ~BusinessThreadImpl();

  static void ShutdownThreadPool();

 protected:
  virtual void Init() OVERRIDE;
  virtual void CleanUp() OVERRIDE;
  virtual void ThreadMain() OVERRIDE;
 private:
  // We implement all the functionality of the public BusinessThread
  // functions, but state is stored in the BusinessThreadImpl to keep
  // the API cleaner. Therefore make BusinessThread a friend class.
  friend class BusinessThread;

  static bool PostTaskHelper(
      BusinessThreadID identifier,
      const tracked_objects::Location& from_here,
      const base::Closure& task,
      base::TimeDelta delay,
      bool nestable);

  // Common initialization code for the constructors.
  void Initialize();

  // The identifier of this thread.  Only one thread can exist with a given
  // identifier at a given time.
  BusinessThreadID identifier_;
};

#endif  // CONTENT_BROWSER_BROWSER_THREAD_IMPL_H_
