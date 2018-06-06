#ifndef app_state_observer_hpp
#define app_state_observer_hpp

#include "common/observer_wrapper.hpp"
#include "base/threading/non_thread_safe.h"

namespace common {

enum AppState {
  APP_STATE_FOREGROUND,
  APP_STATE_BACKGROUND,
};

class AppStateObserver {
 public:
  virtual void OnAppStateChanged(AppState state) = 0;
};

class AppStateProxy : public ObserverWrapper<AppStateObserver>, public base::NonThreadSafe {
 public:
  static AppStateProxy* Create();
  virtual ~AppStateProxy() {}
  
  virtual bool AppIsBackground() = 0;

 protected:
  AppStateProxy() {}
};

}

#endif /* app_state_observer_hpp */
