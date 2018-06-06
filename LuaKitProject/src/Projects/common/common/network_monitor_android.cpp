#include "network_monitor.h"

//
// implementation of class NetworkMonitorAndroid
//
class NetworkMonitorAndroid : public NetworkMonitor {
public:  
  explicit NetworkMonitorAndroid() : NetworkMonitor() {
  }
  
  virtual ~NetworkMonitorAndroid() {
  }
  
  void Start() {
    
  }
};

NetworkMonitor* NetworkMonitor::Create() {
  return new NetworkMonitorAndroid();
}
