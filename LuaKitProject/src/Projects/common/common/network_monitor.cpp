#include "network_monitor.h"

#include "base/bind.h"
#include "base/logging.h"
#include "common/notification_service.h"
#include "common/business_client_thread.h"
#include "common/base_lambda_support.h"
#include "common/network_util.h"

//
// implementation of class NetworkMonitor
//
NetworkMonitor::NetworkMonitor() :
  last_notify_time_(base::Time::Now()) {
}

void NetworkMonitor::SetNetworkChanged(bool reachable) {
  base::Time now = base::Time::Now();
  
  // 如果和上次通知间隔大于2秒就直接通知
  if (now - last_notify_time_ > base::TimeDelta::FromSeconds(2)) {
    Notify(reachable);
  }
  // 小于2秒，3秒后发通知
  else {
    delay_notify_timer_.reset(new base::OneShotTimer<NetworkMonitor>());
    delay_notify_timer_->Start(FROM_HERE, base::TimeDelta::FromSeconds(3), base::Bind(&NetworkMonitor::Notify, base::Unretained(this), reachable));
  }
}

void NetworkMonitor::Notify(bool reachable) {
  last_notify_time_ = base::Time::Now();
  
  delay_notify_timer_.reset();

  LOG(WARNING) << "NetworkMonitor 监控发现网络变化! reachable: " << reachable << ", type " << NetworkUtil::NetWorkType();
  content::NotificationService::current()->Notify(
    NOTIFICATION_NETWORK_CHANGED,
    content::Source<NetworkMonitor>(this),
    content::Details<bool>(&reachable));
  // 在IO线程再发一次同样的通知
  BusinessThread::PostTask(BusinessThread::IO, FROM_HERE, base::BindLambda([=](){
    bool reachable2 = reachable;
    content::NotificationService::current()->Notify(
      NOTIFICATION_NETWORK_CHANGED,
      content::Source<NetworkMonitor>(this),
      content::Details<bool>(&reachable2));
  }));
}
