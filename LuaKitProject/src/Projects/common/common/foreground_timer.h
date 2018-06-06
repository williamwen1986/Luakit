#ifndef forground_timer_h
#define forground_timer_h

#include "base/bind.h"
#include "base/callback.h"
#include "base/timer/timer.h"
#include "base/memory/singleton.h"
#include "base/observer_list_threadsafe.h"
#include <vector>

namespace base {

/*
template <class Receiver, bool kIsRepeating>
class BaseTimerMethodPointer : public Timer {
 public:
  typedef void (Receiver::*ReceiverMethod)();

  // This is here to work around the fact that Timer::Start is "hidden" by the
  // Start definition below, rather than being overloaded.
  // TODO(tim): We should remove uses of BaseTimerMethodPointer::Start below
  // and convert callers to use the base::Closure version in Timer::Start,
  // see bug 148832.
  using Timer::Start;

  BaseTimerMethodPointer() : Timer(kIsRepeating, kIsRepeating) {}

  // Start the timer to run at the given |delay| from now. If the timer is
  // already running, it will be replaced to call a task formed from
  // |reviewer->*method|.
  void Start(const tracked_objects::Location& posted_from,
             TimeDelta delay,
             Receiver* receiver,
             ReceiverMethod method) {
    Timer::Start(posted_from, delay,
                 base::Bind(method, base::Unretained(receiver)));
  }
};
*/
  
class AppStateObserver {
public:
  virtual ~AppStateObserver() { }
  virtual void OnForeground() = 0;
  virtual void OnBackground() = 0;
};

class AppStateManager {
public:
  static AppStateManager* GetInstance();
  
  void EnterForeground() {
    if (!is_foreground_) {
      is_foreground_ = true;
      observers_->Notify(&AppStateObserver::OnForeground);
      LOG(WARNING) << "EnterForeground";
    }
  }
  
  void EnterBackground() {
    if (is_foreground_) {
      is_foreground_ = false;
      observers_->Notify(&AppStateObserver::OnBackground);
      LOG(WARNING) << "EnterBackground";
    }
  } 
  
  bool IsForeground() {
    return is_foreground_;
  }
  
  void AddObserver(AppStateObserver* obs) {
    observers_->AddObserver(obs);
  }
  
  void RemoveObserver(AppStateObserver* obs) {
    observers_->RemoveObserver(obs);
  }
  
  static void AddStaticObserver(AppStateObserver* obs) {
    StaticObservers().push_back(obs);
  }
  
protected:
  AppStateManager() : observers_(new ObserverListThreadSafe<AppStateObserver>), is_foreground_(true) {
    for (auto observer : StaticObservers()) {
      AddObserver(observer);
    }
    StaticObservers().clear();
  }
  static std::vector<AppStateObserver *>& StaticObservers() {
    static std::vector<AppStateObserver *> list;
    return list;
  }
private:
  friend struct DefaultSingletonTraits<AppStateManager>;
  scoped_refptr<ObserverListThreadSafe<AppStateObserver> > observers_;
  bool is_foreground_ = false;
};

class ForegroundTimer : public base::Timer, public base::AppStateObserver {
 public:
  static ForegroundTimer* Create(bool execute_once_when_enter_background = true) {
    return new ForegroundTimer(execute_once_when_enter_background);
  }
  
  virtual ~ForegroundTimer() {
    AppStateManager::GetInstance()->RemoveObserver(this);
  }

  virtual void OnForeground() {
    if (!user_task_.is_null()) {
      Reset();
    }
  };

  virtual void OnBackground() {
    if (is_running_ && execute_once_when_enter_background_ && !user_task_.is_null()) {
      base::Closure task = user_task_;
      Stop();
      task.Run();
    } else {
      Stop();
    }
  };

  template <class Receiver>
  void Start(const tracked_objects::Location& posted_from,
             TimeDelta delay,
             Receiver* receiver,
             void (Receiver::*method)()) {
    Timer::Start(posted_from, delay,
                 base::Bind(method, base::Unretained(receiver)));
    if (!AppStateManager::GetInstance()->IsForeground()) {
      OnBackground();
    }
  }

 protected:
  ForegroundTimer(bool execute_once_when_enter_background = true) : Timer(true, true), execute_once_when_enter_background_(execute_once_when_enter_background) {
    AppStateManager::GetInstance()->AddObserver(this);
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(ForegroundTimer);
  bool execute_once_when_enter_background_; // 如果设置为true，进入后台的时候不管到没到点都立即执行一次timer，然后再关闭掉
};
  
class StaticAppStateObserver : public AppStateObserver {
public:
  StaticAppStateObserver(const base::Callback<void(bool foreground)>& handler) {
    handler_ = handler;
    AppStateManager::AddStaticObserver(this);
  }
  virtual ~StaticAppStateObserver() {
    AppStateManager::GetInstance()->RemoveObserver(this);
  }
  virtual void OnForeground() { handler_.Run(true); }
  virtual void OnBackground() { handler_.Run(false); }
  const base::Callback<void(bool forground)>& Handler() { return handler_; }
private:
  base::Callback<void(bool forground)> handler_;
};

}

#endif /* forground_timer_h */
