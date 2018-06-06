// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/business_client_process_sub_thread.h"

#include "base/debug/leak_tracker.h"
#include "base/threading/thread_restrictions.h"
#include "build/build_config.h"
#include "common/notification_service.h"
#include "common/notification_service_impl.h"
//#include "net/url_request/url_fetcher.h"

BusinessProcessSubThread::BusinessProcessSubThread(BusinessThreadID identifier, const char * threadName)
    : BusinessThreadImpl(identifier,threadName)/*,
      notification_service_(NULL)*/ {
}

BusinessProcessSubThread::~BusinessProcessSubThread() {
  Stop();
}

void BusinessProcessSubThread::Init() {
  notification_service_ = new content::NotificationServiceImpl();

  BusinessThreadImpl::Init();

//  if (BusinessThread::CurrentlyOn(BusinessThread::IO)) {
//    // Though this thread is called the "IO" thread, it actually just routes
//    // messages around; it shouldn't be allowed to perform any blocking disk
//    // I/O.
//    base::ThreadRestrictions::SetIOAllowed(false);
//    base::ThreadRestrictions::DisallowWaiting();
//  }
}

void BusinessProcessSubThread::CleanUp() {
//  if (BusinessThread::CurrentlyOn(BusinessThread::IO))
//    IOThreadPreCleanUp();

  BusinessThreadImpl::CleanUp();

  delete notification_service_;
  notification_service_ = NULL;
}

void BusinessProcessSubThread::IOThreadPreCleanUp() {
  // Kill all things that might be holding onto
  // net::URLRequest/net::URLRequestContexts.

  // Destroy all URLRequests started by URLFetchers.
//  net::URLFetcher::CancelAll();

//  IndexedDBKeyUtilityClient::Shutdown();

  // If any child processes are still running, terminate them and
  // and delete the BrowserChildProcessHost instances to release whatever
  // IO thread only resources they are referencing.
//  BrowserChildProcessHostImpl::TerminateAll();
}
