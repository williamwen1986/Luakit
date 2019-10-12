// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_BROWSER_PROCESS_SUB_THREAD_H_
#define CONTENT_BROWSER_BROWSER_PROCESS_SUB_THREAD_H_
#pragma once

#include "base/basictypes.h"
#include "common/business_client_thread_impl.h"

namespace content {
  class NotificationService;
}

// ----------------------------------------------------------------------------
// BusinessProcessSubThread
//
// This simple thread object is used for the specialized threads that the
// FoxmailProcess spins up.
//
// Applications must initialize the COM library before they can call
// COM library functions other than CoGetMalloc and memory allocation
// functions, so this class initializes COM for those users.
class BusinessProcessSubThread : public BusinessThreadImpl {
 public:
  explicit BusinessProcessSubThread(BusinessThreadID identifier, const char * threadName);
  virtual ~BusinessProcessSubThread();

 protected:
  virtual void Init() OVERRIDE;
  virtual void CleanUp() OVERRIDE;

 private:
  // These methods encapsulate cleanup that needs to happen on the IO thread
  // before we call the embedder's CleanUp function.
  void IOThreadPreCleanUp();

  // Each specialized thread has its own notification service.
  // Note: We don't use scoped_ptr because the destructor runs on the wrong
  // thread.
  content::NotificationService* notification_service_;

  DISALLOW_COPY_AND_ASSIGN(BusinessProcessSubThread);
};

#endif  // CONTENT_BROWSER_BROWSER_PROCESS_SUB_THREAD_H_
