//
//  forground_timer.cpp
//  common
//
//  Created by zhd32 on 15/12/10.
//  Copyright © 2015年 rdgz. All rights reserved.
//

#include "common/foreground_timer.h"

namespace base {
  
AppStateManager* base::AppStateManager::GetInstance() {
  return Singleton<AppStateManager>::get();
}
  
}
