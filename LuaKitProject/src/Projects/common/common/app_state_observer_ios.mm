#import <UIKit/UIKit.h>

#include "common/app_state_observer.h"

@interface AppStateNotificationReceiver : NSObject

@property (nonatomic, assign) common::AppStateProxy* delegate;

@end

@implementation AppStateNotificationReceiver

- (id)init {
  self = [super init];

  if (self != nil) {
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onForeGround) name:UIApplicationWillEnterForegroundNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onBackGround) name:UIApplicationDidEnterBackgroundNotification object:nil];
  }

  return self;
}

- (void)dealloc {
  [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)onForeGround {
  if (self.delegate) {
    self.delegate->NotifyObserver(&common::AppStateObserver::OnAppStateChanged, common::APP_STATE_FOREGROUND);
  }
}

- (void)onBackGround {
  if (self.delegate) {
    self.delegate->NotifyObserver(&common::AppStateObserver::OnAppStateChanged, common::APP_STATE_BACKGROUND);
  }
}

- (void)reset {
  [[NSNotificationCenter defaultCenter] removeObserver:self];
  self.delegate = nullptr;
}

@end

namespace common {

class AppStateProxyImpl : public AppStateProxy {
 public:
  AppStateProxyImpl();

  ~AppStateProxyImpl();
  
  virtual bool AppIsBackground() override;

 private:
  AppStateNotificationReceiver* receiver_;
};

AppStateProxy* AppStateProxy::Create() {
  return new AppStateProxyImpl();
}

AppStateProxyImpl::AppStateProxyImpl() {
  receiver_ = [[AppStateNotificationReceiver alloc] init];
  receiver_.delegate = this;
}

AppStateProxyImpl::~AppStateProxyImpl() {
  [receiver_ reset];
}
  
bool AppStateProxyImpl::AppIsBackground() {
  return [[UIApplication sharedApplication] applicationState] == UIApplicationStateBackground;
}

}
