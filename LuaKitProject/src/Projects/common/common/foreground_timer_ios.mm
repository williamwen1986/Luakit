#import <UIKit/UIKit.h>

#include "common/foreground_timer.h"
#include "common/base_lambda_support.h"
#include "base/message_loop/message_loop_proxy.h"

@interface AppStateProxy : NSObject {
  base::AppStateManager* manager_;
}
- (id)initWithManager:(base::AppStateManager*)manager;
@end

@implementation AppStateProxy

- (id)initWithManager:(base::AppStateManager*)manager {
  self = [super init];
  if (self) {
    manager_ = manager;
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onBackground) name:UIApplicationDidEnterBackgroundNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onForeground) name:UIApplicationWillEnterForegroundNotification object:nil];
  }
  return self;
}

- (void)dealloc {
  [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)onForeground {
  manager_->EnterForeground();
}

-(void)onBackground {
  manager_->EnterBackground();
}

@end

namespace base {

class AppStateManagerIOS : public AppStateManager {
public:
  static AppStateManagerIOS* GetInstance() {
    return Singleton<AppStateManagerIOS>::get();
  }

  virtual ~AppStateManagerIOS() {
    proxy_ = nil;
  }
protected:
  AppStateManagerIOS() : AppStateManager() {
    proxy_ = [[AppStateProxy alloc] initWithManager:this];
  }
private:
  friend struct DefaultSingletonTraits<AppStateManagerIOS>;
   AppStateProxy *proxy_;
};

AppStateManager* base::AppStateManager::GetInstance() {
  return AppStateManagerIOS::GetInstance();
}

}
