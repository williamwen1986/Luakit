// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_BROWSER_THREAD_H_
#define CONTENT_PUBLIC_BROWSER_BROWSER_THREAD_H_
#pragma once

#include <string>

#include "base/basictypes.h"
#include "base/callback.h"
#include "base/location.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/task_runner_util.h"
#include "base/time/time.h"
extern "C" {
#include "lstate.h"
}


#if defined(UNIT_TEST)
#include "base/logging.h"
#endif  // UNIT_TEST

namespace base {
class MessageLoop;
class SequencedWorkerPool;
class Thread;
}

namespace content {
  class BusinessThreadDelegate;
}

class BusinessThreadImpl;

typedef uint32_t BusinessThreadID ;

///////////////////////////////////////////////////////////////////////////////
// BusinessThread
//
// Utility functions for threads that are known by a browser-wide
// name.  For example, there is one IO thread for the entire browser
// process, and various pieces of code find it useful to retrieve a
// pointer to the IO thread's message loop.
//
// Invoke a task by thread ID:
//
//   BusinessThread::PostTask(BusinessThread::IO, FROM_HERE, task);
//
// The return value is false if the task couldn't be posted because the target
// thread doesn't exist.  If this could lead to data loss, you need to check the
// result and restructure the code to ensure it doesn't occur.
//
// This class automatically handles the lifetime of different threads.
// It's always safe to call PostTask on any thread.  If it's not yet created,
// the task is deleted.  There are no race conditions.  If the thread that the
// task is posted to is guaranteed to outlive the current thread, then no locks
// are used.  You should never need to cache pointers to MessageLoops, since
// they're not thread safe.
class BusinessThread {
 public:
  // An enumeration of the well-known threads.
  // NOTE: threads must be listed in the order of their life-time, with each
  // thread outliving every other thread below it.
  enum ID {
    // The main thread in the browser.
    UI,

    // This is the thread that interacts with the database.
    DB,

    LOGIC,
    // This is the "main" thread for WebKit within the browser process when
    // NOT in --single-process mode.
    // Deprecated: Do not design new code to use this thread; see
    // http://crbug.com/106839
//    WEBKIT_DEPRECATED,
      
    

    // This is the thread that interacts with the file system.
    FILE,

    // 网络通信线程
    IO,

    // NOTE: do not add new threads here that are only used by a small number of
    // files. Instead you should just use a Thread class and pass its
    // MessageLoopProxy around. Named threads there are only for threads that
    // are used in many places.

    // This identifier does not represent a thread.  Instead it counts the
    // number of well-known threads.  Insert new well-known threads before this
    // identifier.
    ID_COUNT
  };

  // These are the same methods in message_loop.h, but are guaranteed to either
  // get posted to the MessageLoop if it's still alive, or be deleted otherwise.
  // They return true iff the thread existed and the task was posted.  Note that
  // even if the task is posted, there's no guarantee that it will run, since
  // the target thread may already have a Quit message in its queue.
  static bool PostTask(BusinessThreadID identifier,
                       const tracked_objects::Location& from_here,
                       const base::Closure& task);
  static bool PostDelayedTask(BusinessThreadID identifier,
                              const tracked_objects::Location& from_here,
                              const base::Closure& task,
                              int64 delay_ms);
  static bool PostDelayedTask(BusinessThreadID identifier,
                              const tracked_objects::Location& from_here,
                              const base::Closure& task,
                              base::TimeDelta delay);
  static bool PostNonNestableTask(BusinessThreadID identifier,
                                  const tracked_objects::Location& from_here,
                                  const base::Closure& task);
  static bool PostNonNestableDelayedTask(
      BusinessThreadID identifier,
      const tracked_objects::Location& from_here,
      const base::Closure& task,
      int64 delay_ms);
  static bool PostNonNestableDelayedTask(
      BusinessThreadID identifier,
      const tracked_objects::Location& from_here,
      const base::Closure& task,
      base::TimeDelta delay);

  static bool PostTaskAndReply(
      BusinessThreadID identifier,
      const tracked_objects::Location& from_here,
      const base::Closure& task,
      const base::Closure& reply);

  template <typename ReturnType>
  static bool PostTaskAndReplyWithResult(
      BusinessThreadID identifier,
      const tracked_objects::Location& from_here,
      const base::Callback<ReturnType(void)>& task,
      const base::Callback<void(ReturnType)>& reply) {
    scoped_refptr<base::MessageLoopProxy> message_loop_proxy =
        GetMessageLoopProxyForThread(identifier);
    return base::PostTaskAndReplyWithResult<ReturnType>(
        message_loop_proxy.get(), from_here, task, reply);
  }

  template <class T>
  static bool DeleteSoon(BusinessThreadID identifier,
                         const tracked_objects::Location& from_here,
                         const T* object) {
    return GetMessageLoopProxyForThread(identifier)->DeleteSoon(
        from_here, object);
  }

  template <class T>
  static bool ReleaseSoon(BusinessThreadID identifier,
                          const tracked_objects::Location& from_here,
                          const T* object) {
    return GetMessageLoopProxyForThread(identifier)->ReleaseSoon(
        from_here, object);
  }

  // Simplified wrappers for posting to the blocking thread pool. Use this
  // for doing things like blocking I/O.
  //
  // The first variant will run the task in the pool with no sequencing
  // semantics, so may get run in parallel with other posted tasks. The second
  // variant will all post a task with no sequencing semantics, and will post a
  // reply task to the origin TaskRunner upon completion.  The third variant
  // provides sequencing between tasks with the same sequence token name.
  //
  // These tasks are guaranteed to run before shutdown.
  //
  // If you need to provide different shutdown semantics (like you have
  // something slow and noncritical that doesn't need to block shutdown),
  // or you want to manually provide a sequence token (which saves a map
  // lookup and is guaranteed unique without you having to come up with a
  // unique string), you can access the sequenced worker pool directly via
  // GetBlockingPool().
  static bool PostBlockingPoolTask(const tracked_objects::Location& from_here,
                                   const base::Closure& task);
  static bool PostBlockingPoolTaskAndReply(
      const tracked_objects::Location& from_here,
      const base::Closure& task,
      const base::Closure& reply);
  static bool PostBlockingPoolSequencedTask(
      const std::string& sequence_token_name,
      const tracked_objects::Location& from_here,
      const base::Closure& task);

  // Returns the thread pool used for blocking file I/O. Use this object to
  // perform random blocking operations such as file writes or querying the
  // Windows registry.
  static base::SequencedWorkerPool* GetBlockingPool();

  // Callable on any thread.  Returns whether the given ID corresponds to a well
  // known thread.
  static bool IsWellKnownThread(BusinessThreadID identifier);

  // Callable on any thread.  Returns whether you're currently on a particular
  // thread.
  static bool CurrentlyOn(BusinessThreadID identifier);

  // Callable on any thread.  Returns whether the threads message loop is valid.
  // If this returns false it means the thread is in the process of shutting
  // down.
  static bool IsMessageLoopValid(BusinessThreadID identifier);

  // If the current message loop is one of the known threads, returns true and
  // sets identifier to its ID.  Otherwise returns false.
  static bool GetCurrentThreadIdentifier(BusinessThreadID* identifier);
    
  static bool GetThreadIdentifierByName(BusinessThreadID* identifier, std::string name);
    
  static lua_State * GetCurrentThreadLuaState();
    
  static int getThreadCount();
  // Callers can hold on to a refcounted MessageLoopProxy beyond the lifetime
  // of the thread.
  static scoped_refptr<base::MessageLoopProxy> GetMessageLoopProxyForThread(
      BusinessThreadID identifier);

  // Returns a pointer to the thread's message loop, which will become
  // invalid during shutdown, so you probably shouldn't hold onto it.
  //
  // This must not be called before the thread is started, or after
  // the thread is stopped, or it will DCHECK.
  //
  // Ownership remains with the BusinessThread implementation, so you
  // must not delete the pointer.
  static base::MessageLoop* UnsafeGetMessageLoopForThread(BusinessThreadID identifier);

  // Sets the delegate for the specified BusinessThread.
  //
  // Only one delegate may be registered at a time.  Delegates may be
  // unregistered by providing a NULL pointer.
  //
  // If the caller unregisters a delegate before CleanUp has been
  // called, it must perform its own locking to ensure the delegate is
  // not deleted while unregistering.
  static void SetDelegate(BusinessThreadID identifier, content::BusinessThreadDelegate* delegate);

  // Use these templates in conjuction with RefCountedThreadSafe when you want
  // to ensure that an object is deleted on a specific thread.  This is needed
  // when an object can hop between threads (i.e. IO -> FILE -> IO), and thread
  // switching delays can mean that the final IO tasks executes before the FILE
  // task's stack unwinds.  This would lead to the object destructing on the
  // FILE thread, which often is not what you want (i.e. to unregister from
  // NotificationService, to notify other objects on the creating thread etc).
  template<BusinessThreadID thread>
  struct DeleteOnThread {
    template<typename T>
    static void Destruct(const T* x) {
      if (CurrentlyOn(thread)) {
        delete x;
      } else {
        if (!DeleteSoon(thread, FROM_HERE, x)) {
#if defined(UNIT_TEST)
          // Only logged under unit testing because leaks at shutdown
          // are acceptable under normal circumstances.
          LOG(ERROR) << "DeleteSoon failed on thread " << thread;
#endif  // UNIT_TEST
        }
      }
    }
  };

  // Sample usage:
  // class Foo
  //     : public base::RefCountedThreadSafe<
  //           Foo, BusinessThread::DeleteOnIOThread> {
  //
  // ...
  //  private:
  //   friend struct BusinessThread::DeleteOnThread<BusinessThread::IO>;
  //   friend class DeleteTask<Foo>;
  //
  //   ~Foo();
  struct DeleteOnUIThread : public DeleteOnThread<UI> { };
  struct DeleteOnIOThread : public DeleteOnThread<IO> { };
  struct DeleteOnFileThread : public DeleteOnThread<FILE> { };
  struct DeleteOnDBThread : public DeleteOnThread<DB> { };
  struct DeleteOnLogicThread : public DeleteOnThread<LOGIC> {};
//  struct DeleteOnWebKitThread : public DeleteOnThread<WEBKIT_DEPRECATED> { };

 private:
  friend class BusinessThreadImpl;

  BusinessThread() {}
  DISALLOW_COPY_AND_ASSIGN(BusinessThread);
};

#endif  // CONTENT_PUBLIC_BROWSER_BROWSER_THREAD_H_
