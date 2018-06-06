#include "network_util.h"

//int NetworkUtil::NetWorkType() {
//  
//}

void NetworkUtil::GetHostPortAndParamForHttp(const std::string& url, std::string& host, std::string& port, std::string& param) {
  host.clear(); port.clear();
  if(!url.empty()) {
    size_t begin = url.find("://");
    if(begin != std::string::npos) {
      std::string scheam = url.substr(0, begin);
      if(scheam == "http") {
        port = "80";
      } else if(scheam == "https"){
        port = "443";
      }
      begin += 3;
      if(begin < url.size()) {
        size_t end = url.find("/", begin);
        if(end != std::string::npos) {
          host = url.substr(begin, end-begin);
          param = url.substr(end);
        }
      }
    }
  }
}
