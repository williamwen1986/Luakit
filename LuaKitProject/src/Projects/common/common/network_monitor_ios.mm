#import "common/Reachability.h"

#include "base/bind.h"
#include "network_monitor.h"

NetworkStatus CURRENT_NETWORK_STATUS = ReachableViaWiFi;

enum NetType {
    NETTYPE_UNKNOWN = 0,
    NETTYPE_WIFI    = 1,
    NETTYPE_CELL    = 2
};

static std::atomic<NetType> g_net_type;

void updateNetType() {
    switch (CURRENT_NETWORK_STATUS) {
        case ReachableViaWiFi:
            g_net_type = NETTYPE_WIFI;
            break;
        case ReachableViaWWAN:
        case ReachableVia2G:
        case ReachableVia3G:
        case ReachableVia4G:
            g_net_type = NETTYPE_CELL;
            break;
        default:
            g_net_type = NETTYPE_UNKNOWN;
            break;
    }
}

@interface WWNetworkMonitor : NSObject {
  Reachability *reachability_;
  NetworkMonitor* monitor_;
}
@end

@implementation WWNetworkMonitor
-(id)initWithCdnMonitor:(NetworkMonitor*)cdnMonitor {
  self = [super init];
  if(self != nil) {
    monitor_ = cdnMonitor;
    reachability_ = [Reachability reachabilityForInternetConnection];
    CURRENT_NETWORK_STATUS = reachability_.currentReachabilityStatus;
    updateNetType();
  }
  
  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(reachabilityChanged:)
                                               name:kReachabilityChangedNotification
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
  CURRENT_NETWORK_STATUS = reachability_.currentReachabilityStatus;
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
