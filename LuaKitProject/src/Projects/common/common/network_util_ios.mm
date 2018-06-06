#import <CoreTelephony/CTTelephonyNetworkInfo.h>

#include "network_util.h"

#import "Reachability.h"

extern NetworkStatus CURRENT_NETWORK_STATUS;

int NetworkUtil::NetWorkType() {
  const int k2G = 1;
  const int kNonWiFi = 2;
  const int k3G = 3;
  const int kWiFi = 4;
  const int k4G = 5;
  
  //客户端网络类型(1=2G,2=非wifi,3=3G,4=wifi,5=4G)
  NetworkStatus status = CURRENT_NETWORK_STATUS;
  
  if(status == NotReachable) {
    return kNonWiFi;
  } else if (status == ReachableViaWiFi) {
    return kWiFi;
  } else if (status == ReachableViaWWAN) {
    CTTelephonyNetworkInfo *netinfo = [[CTTelephonyNetworkInfo alloc] init];
    if ([netinfo.currentRadioAccessTechnology isEqualToString:CTRadioAccessTechnologyGPRS])
      return k2G;
    else if ([netinfo.currentRadioAccessTechnology isEqualToString:CTRadioAccessTechnologyEdge])
      return k2G;
    else if ([netinfo.currentRadioAccessTechnology isEqualToString:CTRadioAccessTechnologyWCDMA])
      return k3G;
    else if ([netinfo.currentRadioAccessTechnology isEqualToString:CTRadioAccessTechnologyHSDPA])
      return k3G;
    else if ([netinfo.currentRadioAccessTechnology isEqualToString:CTRadioAccessTechnologyHSUPA])
      return k3G;
    else if ([netinfo.currentRadioAccessTechnology isEqualToString:CTRadioAccessTechnologyCDMA1x])
      return k2G;
    else if ([netinfo.currentRadioAccessTechnology isEqualToString:CTRadioAccessTechnologyCDMAEVDORev0])
      return k3G;
    else if ([netinfo.currentRadioAccessTechnology isEqualToString:CTRadioAccessTechnologyCDMAEVDORevA])
      return k3G;
    else if ([netinfo.currentRadioAccessTechnology isEqualToString:CTRadioAccessTechnologyCDMAEVDORevB])
      return k3G;
    else if ([netinfo.currentRadioAccessTechnology isEqualToString:CTRadioAccessTechnologyeHRPD])
      return k3G;
    else if ([netinfo.currentRadioAccessTechnology isEqualToString:CTRadioAccessTechnologyLTE])
      return k4G;
  }
  
  return kNonWiFi;
}
