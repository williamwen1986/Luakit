#pragma once

#include <string>

class NetworkUtil {
 public:
  static int NetWorkType();
  
  static void GetHostPortAndParamForHttp(const std::string& url, std::string& host, std::string& port, std::string& param);
  
}; // class NetworkUtil
