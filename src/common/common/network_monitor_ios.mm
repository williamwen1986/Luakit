#import "common/LReachability.h"

#include "base/bind.h"
#include "network_monitor.h"

LNetworkStatus CURRENT_NETWORK_STATUS = LReachableViaWiFi;

enum NetType {
    NETTYPE_UNKNOWN = 0,
    NETTYPE_WIFI    = 1,
    NETTYPE_CELL    = 2
};

static std::atomic<NetType> g_net_type;

void updateNetType() {
    switch (CURRENT_NETWORK_STATUS) {
        case LReachableViaWiFi:
            g_net_type = NETTYPE_WIFI;
            break;
        case LReachableViaWWAN:
        case LReachableVia2G:
        case LReachableVia3G:
        case LReachableVia4G:
            g_net_type = NETTYPE_CELL;
            break;
        default:
            g_net_type = NETTYPE_UNKNOWN;
            break;
    }
}

@interface WWNetworkMonitor : NSObject {
  LReachability *reachability_;
  NetworkMonitor* monitor_;
}
@end

@implementation WWNetworkMonitor
-(id)initWithCdnMonitor:(NetworkMonitor*)cdnMonitor {
  self = [super init];
  if(self != nil) {
    monitor_ = cdnMonitor;
    reachability_ = [LReachability reachabilityForInternetConnection];
    CURRENT_NETWORK_STATUS = reachability_.currentLReachabilityStatus;
    updateNetType();
  }
  
  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(reachabilityChanged:)
                                               name:kLReachabilityChangedNotification
                                             object:reachability_];
  [reachability_ startNotifier];
  
  return self;
}

-(void)dealloc {
  [reachability_ stopNotifier];
  [[NSNotificationCenter defaultCenter] removeObserver:self];
}

-(void)reachabilityChanged:(NSNotification*)notification {
  NSNumber* reachable = [[notification userInfo] objectForKey:@"reachable"];
  CURRENT_NETWORK_STATUS = reachability_.currentLReachabilityStatus;
  updateNetType();
  monitor_->SetNetworkChanged(reachable.boolValue);
}
@end


//
// implementation of class NetworkMonitorIOS
//
class NetworkMonitorIOS : public NetworkMonitor {
public:
  explicit NetworkMonitorIOS() : NetworkMonitor() {
    Start();
  }
  virtual ~NetworkMonitorIOS() {}
  
  void Start() {
    network_monitor_ = [[WWNetworkMonitor alloc] initWithCdnMonitor:this];
  }
  
protected:
  WWNetworkMonitor* network_monitor_;
};

NetworkMonitor* NetworkMonitor::Create() {
  return new NetworkMonitorIOS();
}
