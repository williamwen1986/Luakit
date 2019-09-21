// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/business_client_thread_impl.h"

#include <string>

#include "base/atomicops.h"
#include "base/bind.h"
#include "base/compiler_specific.h"
#include "base/lazy_instance.h"
#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/threading/sequenced_worker_pool.h"
#include "base/threading/thread_restrictions.h"
#include "common/business_client_thread_delegate.h"
#include "tools/lua_helpers.h"
extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lstate.h"
}


namespace {

struct BusinessThreadGlobals {
  BusinessThreadGlobals()
      : blocking_pool(new base::SequencedWorkerPool(3, "BusinessBlocking")) {
  }

  // This lock protects |threads|. Do not read or modify that array
  // without holding this lock. Do not block while holding this lock.
  base::Lock lock;

  // This array is protected by |lock|. The threads are not owned by this
  // array. Typically, the threads are owned on the UI thread by
  // content::BusinessMainLoop. BusinessThreadImpl objects remove themselves from
  // this array upon destruction.
  std::vector<BusinessThreadImpl*> threads;
  std::vector<lua_State*> luaStates;
  // Only atomic operations are used on this array. The delegates are not owned
  // by this array, rather by whoever calls BusinessThread::SetDelegate.
  std::vector<content::BusinessThreadDelegate*> thread_delegates;
  const scoped_refptr<base::SequencedWorkerPool> blocking_pool;
};

base::LazyInstance<BusinessThreadGlobals>::Leaky
    g_globals = LAZY_INSTANCE_INITIALIZER;

}  // namespace

BusinessThreadImpl::BusinessThreadImpl(BusinessThreadID identifier,const char * thread_name)
    : Thread(thread_name),
      identifier_(identifier) {
  BusinessThreadGlobals& globals = g_globals.Get();
  globals.threads.push_back(NULL);
  globals.luaStates.push_back(NULL);
  globals.thread_delegates.push_back(NULL);
  Initialize();
}

BusinessThreadImpl::BusinessThreadImpl(BusinessThreadID identifier,
                                     base::MessageLoop* message_loop)
    : Thread(message_loop->thread_name().c_str()),
      identifier_(identifier) {
  BusinessThreadGlobals& globals = g_globals.Get();
  globals.threads.push_back(NULL);
  globals.luaStates.push_back(NULL);
  globals.thread_delegates.push_back(NULL);
  set_message_loop(message_loop);
  Initialize();
}

// static
void BusinessThreadImpl::ShutdownThreadPool() {
  BusinessThreadGlobals& globals = g_globals.Get();
  globals.blocking_pool->Shutdown();
}

void BusinessThreadImpl::Init() {
  BusinessThreadGlobals& globals = g_globals.Get();

  using base::subtle::AtomicWord;
  AtomicWord* storage =
      reinterpret_cast<AtomicWord*>(&globals.thread_delegates[identifier_]);
  AtomicWord stored_pointer = base::subtle::NoBarrier_Load(storage);
  content::BusinessThreadDelegate* delegate =
      reinterpret_cast<content::BusinessThreadDelegate*>(stored_pointer);
  if (delegate)
    delegate->Init();
}

void BusinessThreadImpl::CleanUp() {
  BusinessThreadGlobals& globals = g_globals.Get();

  using base::subtle::AtomicWord;
  AtomicWord* storage =
      reinterpret_cast<AtomicWord*>(&globals.thread_delegates[identifier_]);
  AtomicWord stored_pointer = base::subtle::NoBarrier_Load(storage);
  content::BusinessThreadDelegate* delegate =
      reinterpret_cast<content::BusinessThreadDelegate*>(stored_pointer);

  if (delegate)
    delegate->CleanUp();
}

void BusinessThreadImpl::Initialize() {
  BusinessThreadGlobals& globals = g_globals.Get();
  base::AutoLock lock(globals.lock);
    DCHECK(identifier_ >= 0 && identifier_ < BusinessThread::getThreadCount());
  DCHECK(globals.threads[identifier_] == NULL);
  globals.threads[identifier_] = this;
  if(identifier_ == UI){
      lua_State* luaState = luaL_newstate();
      luaInit(luaState);
      DLOG(INFO) << "UI luaL_newstate" << identifier_;
      globals.luaStates[identifier_] = luaState;
  }
}

BusinessThreadImpl::~BusinessThreadImpl() {
  // All Thread subclasses must call Stop() in the destructor. This is
  // doubly important here as various bits of code check they are on
  // the right BusinessThread.
  Stop();

  BusinessThreadGlobals& globals = g_globals.Get();
  base::AutoLock lock(globals.lock);
  globals.threads[identifier_] = NULL;
#ifndef NDEBUG
  // Double check that the threads are ordered correctly in the enumeration.
  for (int i = identifier_ + 1; i < BusinessThread::getThreadCount(); ++i) {
    DCHECK(!globals.threads[i]) <<
        "Threads must be listed in the reverse order that they die";
  }
#endif
}

void BusinessThreadImpl::ThreadMain(){
   
    BusinessThreadGlobals& globals = g_globals.Get();
    lua_State* luaState = luaL_newstate();
    luaInit(luaState);
    LOG(INFO) << "BusinessThreadImpl::ThreadMain luaL_newstate" << identifier_;
    globals.luaStates[identifier_] = luaState;
    Thread::ThreadMain();
}

// static
bool BusinessThreadImpl::PostTaskHelper(
    BusinessThreadID identifier,
    const tracked_objects::Location& from_here,
    const base::Closure& task,
    base::TimeDelta delay,
    bool nestable) {
  DCHECK(identifier >= 0 && identifier < BusinessThread::getThreadCount());
  // Optimization: to avoid unnecessary locks, we listed the ID enumeration in
  // order of lifetime.  So no need to lock if we know that the other thread
  // outlives this one.
  // Note: since the array is so small, ok to loop instead of creating a map,
  // which would require a lock because std::map isn't thread safe, defeating
  // the whole purpose of this optimization.
  BusinessThreadID current_thread;
  bool guaranteed_to_outlive_target_thread =
      GetCurrentThreadIdentifier(&current_thread) &&
      current_thread <= identifier;

  BusinessThreadGlobals& globals = g_globals.Get();
  if (!guaranteed_to_outlive_target_thread)
    globals.lock.Acquire();

  base::MessageLoop* message_loop = globals.threads[identifier] ?
      globals.threads[identifier]->message_loop() : NULL;
  if (message_loop) {
    if (nestable) {
      message_loop->PostDelayedTask(from_here, task, delay);
    } else {
      message_loop->PostNonNestableDelayedTask(from_here, task, delay);
    }
  }

  if (!guaranteed_to_outlive_target_thread)
    globals.lock.Release();

  return !!message_loop;
}

// An implementation of MessageLoopProxy to be used in conjunction
// with BusinessThread.
class BusinessThreadMessageLoopProxy : public base::MessageLoopProxy {
 public:
  explicit BusinessThreadMessageLoopProxy(BusinessThreadID identifier)
      : id_(identifier) {
  }

  // MessageLoopProxy implementation.
//  virtual bool PostDelayedTask(
//      const tracked_objects::Location& from_here,
//      const base::Closure& task, int64 delay_ms) OVERRIDE {
//    return BusinessThread::PostDelayedTask(id_, from_here, task, delay_ms);
//  }
  virtual bool PostDelayedTask(
      const tracked_objects::Location& from_here,
      const base::Closure& task, base::TimeDelta delay) OVERRIDE {
    return BusinessThread::PostDelayedTask(id_, from_here, task, delay);
  }

//  virtual bool PostNonNestableDelayedTask(
//      const tracked_objects::Location& from_here,
//      const base::Closure& task,
//      int64 delay_ms) OVERRIDE {
//    return BusinessThread::PostNonNestableDelayedTask(id_, from_here, task,
//                                                     delay_ms);
//  }
  virtual bool PostNonNestableDelayedTask(
      const tracked_objects::Location& from_here,
      const base::Closure& task,
      base::TimeDelta delay) OVERRIDE {
    return BusinessThread::PostNonNestableDelayedTask(id_, from_here, task,
                                                     delay);
  }

  virtual bool RunsTasksOnCurrentThread() const OVERRIDE {
    return BusinessThread::CurrentlyOn(id_);
  }

 protected:
  virtual ~BusinessThreadMessageLoopProxy() {}

 private:
  BusinessThreadID id_;
  DISALLOW_COPY_AND_ASSIGN(BusinessThreadMessageLoopProxy);
};

// static
bool BusinessThread::PostBlockingPoolTask(
    const tracked_objects::Location& from_here,
    const base::Closure& task) {
  return g_globals.Get().blocking_pool->PostWorkerTask(from_here, task);
}

bool BusinessThread::PostBlockingPoolTaskAndReply(
    const tracked_objects::Location& from_here,
    const base::Closure& task,
    const base::Closure& reply) {
  return g_globals.Get().blocking_pool->PostTaskAndReply(
      from_here, task, reply);
}

// static
bool BusinessThread::PostBlockingPoolSequencedTask(
    const std::string& sequence_token_name,
    const tracked_objects::Location& from_here,
    const base::Closure& task) {
  return g_globals.Get().blocking_pool->PostNamedSequencedWorkerTask(
      sequence_token_name, from_here, task);
}

// static
base::SequencedWorkerPool* BusinessThread::GetBlockingPool() {
  return g_globals.Get().blocking_pool;
}

// static
bool BusinessThread::IsWellKnownThread(BusinessThreadID identifier) {
  if (g_globals == NULL)
    return false;

  BusinessThreadGlobals& globals = g_globals.Get();
  base::AutoLock lock(globals.lock);
  return (identifier >= 0 && identifier < BusinessThread::getThreadCount() &&
          globals.threads[identifier]);
}

// static
bool BusinessThread::CurrentlyOn(BusinessThreadID identifier) {
  // We shouldn't use MessageLoop::current() since it uses LazyInstance which
  // may be deleted by ~AtExitManager when a WorkerPool thread calls this
  // function.
  // http://crbug.com/63678
  base::ThreadRestrictions::ScopedAllowSingleton allow_singleton;
  BusinessThreadGlobals& globals = g_globals.Get();
  base::AutoLock lock(globals.lock);
  DCHECK(identifier >= 0 && identifier < BusinessThread::getThreadCount());
  return globals.threads[identifier] &&
         globals.threads[identifier]->message_loop() ==
             base::MessageLoop::current();
}

// static
bool BusinessThread::IsMessageLoopValid(BusinessThreadID identifier) {
  if (g_globals == NULL)
    return false;

  BusinessThreadGlobals& globals = g_globals.Get();
  base::AutoLock lock(globals.lock);
  DCHECK(identifier >= 0 && identifier < BusinessThread::getThreadCount());
  return globals.threads[identifier] &&
         globals.threads[identifier]->message_loop();
}

// static
bool BusinessThread::PostTask(BusinessThreadID identifier,
                             const tracked_objects::Location& from_here,
                             const base::Closure& task) {
  return BusinessThreadImpl::PostTaskHelper(
      identifier, from_here, task, base::TimeDelta(), true);
}

// static
bool BusinessThread::PostDelayedTask(BusinessThreadID identifier,
                                    const tracked_objects::Location& from_here,
                                    const base::Closure& task,
                                    int64 delay_ms) {
  return BusinessThreadImpl::PostTaskHelper(
      identifier,
      from_here,
      task,
      base::TimeDelta::FromMilliseconds(delay_ms),
      true);
}

// static
bool BusinessThread::PostDelayedTask(BusinessThreadID identifier,
                                    const tracked_objects::Location& from_here,
                                    const base::Closure& task,
                                    base::TimeDelta delay) {
  return BusinessThreadImpl::PostTaskHelper(
      identifier, from_here, task, delay, true);
}

// static
bool BusinessThread::PostNonNestableTask(
    BusinessThreadID identifier,
    const tracked_objects::Location& from_here,
    const base::Closure& task) {
  return BusinessThreadImpl::PostTaskHelper(
      identifier, from_here, task, base::TimeDelta(), false);
}

// static
bool BusinessThread::PostNonNestableDelayedTask(
    BusinessThreadID identifier,
    const tracked_objects::Location& from_here,
    const base::Closure& task,
    int64 delay_ms) {
  return BusinessThreadImpl::PostTaskHelper(
      identifier,
      from_here,
      task,
      base::TimeDelta::FromMilliseconds(delay_ms),
      false);
}

// static
bool BusinessThread::PostNonNestableDelayedTask(
    BusinessThreadID identifier,
    const tracked_objects::Location& from_here,
    const base::Closure& task,
    base::TimeDelta delay) {
  return BusinessThreadImpl::PostTaskHelper(
      identifier, from_here, task, delay, false);
}

// static
bool BusinessThread::PostTaskAndReply(
    BusinessThreadID identifier,
    const tracked_objects::Location& from_here,
    const base::Closure& task,
    const base::Closure& reply) {
  return GetMessageLoopProxyForThread(identifier)->PostTaskAndReply(from_here,
                                                                    task,
                                                                    reply);
}

// static
bool BusinessThread::GetCurrentThreadIdentifier(BusinessThreadID* identifier) {
  if (g_globals == NULL)
    return false;

  // We shouldn't use MessageLoop::current() since it uses LazyInstance which
  // may be deleted by ~AtExitManager when a WorkerPool thread calls this
  // function.
  // http://crbug.com/63678
  base::ThreadRestrictions::ScopedAllowSingleton allow_singleton;
  base::MessageLoop* cur_message_loop = base::MessageLoop::current();
  BusinessThreadGlobals& globals = g_globals.Get();
  for (int i = 0; i < BusinessThread::getThreadCount(); ++i) {
    if (globals.threads[i] &&
        globals.threads[i]->message_loop() == cur_message_loop) {
      *identifier = globals.threads[i]->identifier_;
      return true;
    }
  }

  return false;
}

bool BusinessThread::GetThreadIdentifierByName(BusinessThreadID* identifier, std::string name) {
    if (g_globals == NULL)
        return false;
    
    // We shouldn't use MessageLoop::current() since it uses LazyInstance which
    // may be deleted by ~AtExitManager when a WorkerPool thread calls this
    // function.
    // http://crbug.com/63678
    base::ThreadRestrictions::ScopedAllowSingleton allow_singleton;
    BusinessThreadGlobals& globals = g_globals.Get();
    for (int i = 0; i < BusinessThread::getThreadCount(); ++i) {
        if (globals.threads[i] &&
            globals.threads[i]->thread_name() == name) {
            *identifier = globals.threads[i]->identifier_;
            return true;
        }
    }
    return false;
}

lua_State* BusinessThread::GetCurrentThreadLuaState(){
    base::MessageLoop* cur_message_loop = base::MessageLoop::current();
    BusinessThreadGlobals& globals = g_globals.Get();
    for (int i = 0; i < BusinessThread::getThreadCount(); ++i) {
        if (globals.threads[i] &&
            globals.threads[i]->message_loop() == cur_message_loop) {
            return globals.luaStates[i];
        }
    }
    return NULL;
}

int BusinessThread::getThreadCount(){
    BusinessThreadGlobals& globals = g_globals.Get();
    return (int)globals.threads.size();
}

// static
scoped_refptr<base::MessageLoopProxy>
BusinessThread::GetMessageLoopProxyForThread(BusinessThreadID identifier) {
  scoped_refptr<base::MessageLoopProxy> proxy(
      new BusinessThreadMessageLoopProxy(identifier));
  return proxy;
}

// static
base::MessageLoop* BusinessThread::UnsafeGetMessageLoopForThread(BusinessThreadID identifier) {
  if (g_globals == NULL)
    return NULL;

  BusinessThreadGlobals& globals = g_globals.Get();
  base::AutoLock lock(globals.lock);
  base::Thread* thread = globals.threads[identifier];
  DCHECK(thread);
  base::MessageLoop* loop = thread->message_loop();
  return loop;
}

// static
void BusinessThread::SetDelegate(BusinessThreadID identifier,
                                content::BusinessThreadDelegate* delegate) {
  using base::subtle::AtomicWord;
  BusinessThreadGlobals& globals = g_globals.Get();
  AtomicWord* storage = reinterpret_cast<AtomicWord*>(
      &globals.thread_delegates[identifier]);
  AtomicWord old_pointer = base::subtle::NoBarrier_AtomicExchange(
      storage, reinterpret_cast<AtomicWord>(delegate));

  // This catches registration when previously registered.
  DCHECK(!delegate || !old_pointer);
}
