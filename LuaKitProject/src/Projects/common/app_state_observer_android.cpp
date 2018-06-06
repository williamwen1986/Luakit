//
//  app_state_observer.cpp
//  common
//
//  Created by zhd32 on 15/12/24.
//  Copyright © 2015年 rdgz. All rights reserved.
//

#include "common/app_state_observer.h"

namespace common {

class AppStateProxyImpl : public AppStateProxy {
 public:
  AppStateProxyImpl();

  ~AppStateProxyImpl();
  
  virtual bool AppIsBackground() override;
 
};

AppStateProxy* AppStateProxy::Create() {
  return new AppStateProxyImpl();
}

AppStateProxyImpl::AppStateProxyImpl() {
  
}

AppStateProxyImpl::~AppStateProxyImpl() {
  
}
  
bool AppStateProxyImpl::AppIsBackground() {
  //todo add jni wrapper
  return true;
}

}
