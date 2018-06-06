#pragma once

#include "base/basictypes.h"
#include "base/bind.h"
#include "base/callback.h"
#include "base/callback_internal.h"
#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "base/message_loop/message_loop.h"

template<typename CB>
class ExcuteOnCallerCallback;

//
// 让一个callback在caller线程执行回调
//
template<typename CB>
CB MakeCallbackOnCallerLoop(CB cb) {
  ExcuteOnCallerCallback<CB> *callback = new ExcuteOnCallerCallback<CB>(cb);
  return callback->GetCB();
}

template<typename CB>
CB MakeCallbackOnSpecificLoop(CB cb, base::MessageLoopProxy* loop) {
  ExcuteOnCallerCallback<CB> *callback = new ExcuteOnCallerCallback<CB>(cb, loop);
  return callback->GetCB();
}

class ExcuteOnCallerCallbackBase {
public:
  virtual ~ExcuteOnCallerCallbackBase() {
    callback_thread_ = NULL;
  }
  
protected:
  ExcuteOnCallerCallbackBase() :
    callback_thread_(base::MessageLoopProxy::current()) {
  }

  ExcuteOnCallerCallbackBase(base::MessageLoopProxy* loop) :
    callback_thread_(loop) {
  }
    
  void DoRun(const base::Closure& forwarded_call) {
    if (callback_thread_ == base::MessageLoopProxy::current()) {
      forwarded_call.Run();
    } else {
      callback_thread_->PostTask(
        FROM_HERE,
        forwarded_call
      );
      callback_thread_->DeleteSoon(
        FROM_HERE,
        this
      );
    }
  }
  
  scoped_refptr<base::MessageLoopProxy> callback_thread_;

private:
  DISALLOW_COPY_AND_ASSIGN(ExcuteOnCallerCallbackBase);
};

template<typename CB>
class ExcuteOnCallerCallback : public ExcuteOnCallerCallbackBase {
public:
  typedef CB CallbackType;
  
  ExcuteOnCallerCallback(const CallbackType& cb) :
    ExcuteOnCallerCallbackBase(),
    callback_(cb) {
  }

  ExcuteOnCallerCallback(const CallbackType& cb, base::MessageLoopProxy* loop) :
    ExcuteOnCallerCallbackBase(loop),
    callback_(cb) {
  }
  
  CallbackType GetCB() {
    return base::Bind(
      &ExcuteOnCallerCallback<CallbackType>::Run,
      base::Unretained(this)
    );
  }
  
  void Run() {
    DoRun(base::Bind(callback_));
  }
  
private:
  CallbackType callback_;
};

template<typename A1>
class ExcuteOnCallerCallback<base::Callback<void(A1)> > : public ExcuteOnCallerCallbackBase {
public:
  typedef base::Callback<void(A1)> CallbackType;
  
  ExcuteOnCallerCallback(const CallbackType& callback)
      : ExcuteOnCallerCallbackBase(),
        callback_(callback) {
    DCHECK(!callback.is_null()) << "Callback must be initialized.";
  }

  ExcuteOnCallerCallback(const CallbackType& cb, base::MessageLoopProxy* loop) :
    ExcuteOnCallerCallbackBase(loop),
    callback_(cb) {
  }
  
  CallbackType GetCB() {
    return base::Bind(
      &ExcuteOnCallerCallback<CallbackType>::Run,
      base::Unretained(this)
    );
  }
  
  void Run(A1 a1) {
    DoRun(base::Bind(callback_, a1));
  }

private:
  CallbackType callback_;
  DISALLOW_COPY_AND_ASSIGN(ExcuteOnCallerCallback);
};

template<typename A1, typename A2>
class ExcuteOnCallerCallback<base::Callback<void(A1, A2)> > : public ExcuteOnCallerCallbackBase {
public:
  typedef base::Callback<void(A1, A2)> CallbackType;
  
  ExcuteOnCallerCallback(const CallbackType& callback)
      : ExcuteOnCallerCallbackBase(),
        callback_(callback) {
    DCHECK(!callback.is_null()) << "Callback must be initialized.";
  }

  ExcuteOnCallerCallback(const CallbackType& callback, base::MessageLoopProxy* loop)
      : ExcuteOnCallerCallbackBase(loop),
        callback_(callback) {
    DCHECK(!callback.is_null()) << "Callback must be initialized.";
  }
  
  CallbackType GetCB() {
    return base::Bind(
      &ExcuteOnCallerCallback<CallbackType>::Run,
      base::Unretained(this)
    );
  }
  
  void Run(A1 a1, A2 a2) {
    DoRun(base::Bind(callback_, a1, a2));
  }

private:
  CallbackType callback_;
  DISALLOW_COPY_AND_ASSIGN(ExcuteOnCallerCallback);
};

template<typename A1, typename A2, typename A3>
class ExcuteOnCallerCallback<base::Callback<void(A1, A2, A3)> > : public ExcuteOnCallerCallbackBase {
public:
  typedef base::Callback<void(A1, A2, A3)> CallbackType;
  
  ExcuteOnCallerCallback(const CallbackType& callback)
      : ExcuteOnCallerCallbackBase(),
        callback_(callback) {
    DCHECK(!callback.is_null()) << "Callback must be initialized.";
  }

  ExcuteOnCallerCallback(const CallbackType& callback, base::MessageLoopProxy* loop)
      : ExcuteOnCallerCallbackBase(loop),
        callback_(callback) {
    DCHECK(!callback.is_null()) << "Callback must be initialized.";
  }
  
  CallbackType GetCB() {
    return base::Bind(
      &ExcuteOnCallerCallback<CallbackType>::Run,
      base::Unretained(this)
    );
  }
  
  void Run(A1 a1, A2 a2, A3 a3) {
    DoRun(base::Bind(callback_, a1, a2, a3));
  }

private:
  CallbackType callback_;
  DISALLOW_COPY_AND_ASSIGN(ExcuteOnCallerCallback);
};
