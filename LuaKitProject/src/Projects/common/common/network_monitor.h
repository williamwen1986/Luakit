#ifndef cdn_network_monitor_h
#define cdn_network_monitor_h

#include <stdint.h>
#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "base/time/time.h"
#include "base/timer/timer.h"

#define NOTIFICATION_NETWORK_CHANGED 10000

class NetworkMonitor {
 public:
  static NetworkMonitor* Create();
  virtual ~NetworkMonitor() {}
  
  void SetNetworkChanged(bool reachable);
  
 protected:
  NetworkMonitor();
  
 private:
  void Notify(bool reachable);
 
  base::Time last_notify_time_;
  
  scoped_ptr<base::Timer> delay_notify_timer_;
 
  DISALLOW_COPY_AND_ASSIGN(NetworkMonitor);
};

#endif /* cdn_network_monitor_h */
