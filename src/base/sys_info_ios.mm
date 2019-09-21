// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/sys_info.h"

#import <UIKit/UIKit.h>
#include <mach/mach.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include "base/logging.h"
#include "base/mac/scoped_mach_port.h"
#include "base/mac/scoped_nsautorelease_pool.h"
#include "base/strings/sys_string_conversions.h"

namespace base {

// static
std::string SysInfo::OperatingSystemName() {
  static dispatch_once_t get_system_name_once;
  static std::string* system_name;
  dispatch_once(&get_system_name_once, ^{
      base::mac::ScopedNSAutoreleasePool pool;
      system_name = new std::string(
          SysNSStringToUTF8([[UIDevice currentDevice] systemName]));
  });
  // Examples of returned value: 'iPhone OS' on iPad 5.1.1
  // and iPhone 5.1.1.
  return *system_name;
}

// static
std::string SysInfo::GetDeviceName() {
  static dispatch_once_t get_device_name_once;
  static std::string* device_name;
  dispatch_once(&get_device_name_once, ^{
      struct utsname systemInfo;
      uname(&systemInfo);
      device_name = new std::string(systemInfo.machine ?: "");
  });
  
  return *device_name;
}

std::string SysInfo::GetReadableDeviceName() {
  std::string name = SysInfo::GetDeviceName();
  
  if (name == "iPhone1,1")    return "iPhone 2G";
  if (name == "iPhone1,2")    return "iPhone 3G";
  if (name == "iPhone2,1")    return "iPhone 3GS";
  if (name == "iPhone3,1")    return "iPhone 4";
  if (name == "iPhone3,2")    return "iPhone 4";
  if (name == "iPhone3,3")    return "iPhone 4 (CDMA)";
  if (name == "iPhone4,1")    return "iPhone 4S";
  if (name == "iPhone5,1")    return "iPhone 5";
  if (name == "iPhone5,2")    return "iPhone 5 (GSM+CDMA)";
  if (name == "iPhone5,3")    return "iPhone 5c (GSM)";
  if (name == "iPhone5,4")    return "iPhone 5c (GSM+CDMA)";
  if (name == "iPhone6,1")    return "iPhone 5s (GSM)";
  if (name == "iPhone6,2")    return "iPhone 5s (GSM+CDMA)";
  if (name == "iPhone7,1")    return "iPhone 6 Plus";
  if (name == "iPhone7,2")    return "iPhone 6";
  if (name == "iPhone8,1")    return "iPhone 6s";
  if (name == "iPhone8,2")    return "iPhone 6s Plus";

  if (name == "iPod1,1")      return "iPod Touch (1 Gen)";
  if (name == "iPod2,1")      return "iPod Touch (2 Gen)";
  if (name == "iPod3,1")      return "iPod Touch (3 Gen)";
  if (name == "iPod4,1")      return "iPod Touch (4 Gen)";
  if (name == "iPod5,1")      return "iPod Touch (5 Gen)";

  if (name == "iPad1,1")      return "iPad";
  if (name == "iPad1,2")      return "iPad 3G";
  if (name == "iPad2,1")      return "iPad 2 (WiFi)";
  if (name == "iPad2,2")      return "iPad 2";
  if (name == "iPad2,3")      return "iPad 2 (CDMA)";
  if (name == "iPad2,4")      return "iPad 2";
  if (name == "iPad2,5")      return "iPad Mini (WiFi)";
  if (name == "iPad2,6")      return "iPad Mini";
  if (name == "iPad2,7")      return "iPad Mini (GSM+CDMA)";

  if (name == "iPad4,4")      return "iPad Mini 2";
  if (name == "iPad4,5")      return "iPad Mini 2";
  if (name == "iPad4,6")      return "iPad Mini 2";

  if (name == "iPad4,7")      return "iPad Mini 3";
  if (name == "iPad4,8")      return "iPad Mini 3";
  if (name == "iPad4,9")      return "iPad Mini 3";

  if (name == "iPad3,1")      return "iPad 3 (WiFi)";
  if (name == "iPad3,2")      return "iPad 3 (GSM+CDMA)";
  if (name == "iPad3,3")      return "iPad 3";
  if (name == "iPad3,4")      return "iPad 4 (WiFi)";
  if (name == "iPad3,5")      return "iPad 4";
  if (name == "iPad3,6")      return "iPad 4 (GSM+CDMA)";
  if (name == "iPad4,1")      return "iPad Air";
  if (name == "iPad4,2")      return "iPad Air";
  if (name == "iPad4,3")      return "iPad Air";
  if (name == "iPad5,3")      return "iPad Air 2";
  if (name == "iPad5,4")      return "iPad Air 2";

  if (name == "i386")         return "Simulator";
  if (name == "x86_64")       return "Simulator";

  return "Unknown";
}

// static
std::string SysInfo::OperatingSystemVersion() {
  static dispatch_once_t get_system_version_once;
  static std::string* system_version;
  dispatch_once(&get_system_version_once, ^{
      base::mac::ScopedNSAutoreleasePool pool;
      system_version = new std::string(
          SysNSStringToUTF8([[UIDevice currentDevice] systemVersion]));
  });
  return *system_version;
}

// static
void SysInfo::OperatingSystemVersionNumbers(int32* major_version,
                                            int32* minor_version,
                                            int32* bugfix_version) {
  base::mac::ScopedNSAutoreleasePool pool;
  std::string system_version = OperatingSystemVersion();
  if (!system_version.empty()) {
    // Try to parse out the version numbers from the string.
    int num_read = sscanf(system_version.c_str(), "%d.%d.%d", major_version,
                          minor_version, bugfix_version);
    if (num_read < 1)
      *major_version = 0;
    if (num_read < 2)
      *minor_version = 0;
    if (num_read < 3)
      *bugfix_version = 0;
  }
}

// static
int64 SysInfo::AmountOfPhysicalMemory() {
  struct host_basic_info hostinfo;
  mach_msg_type_number_t count = HOST_BASIC_INFO_COUNT;
  base::mac::ScopedMachPort host(mach_host_self());
  int result = host_info(host,
                         HOST_BASIC_INFO,
                         reinterpret_cast<host_info_t>(&hostinfo),
                         &count);
  if (result != KERN_SUCCESS) {
    NOTREACHED();
    return 0;
  }
  DCHECK_EQ(HOST_BASIC_INFO_COUNT, count);
  return static_cast<int64>(hostinfo.max_mem);
}

// static
std::string SysInfo::CPUModelName() {
  char name[256];
  size_t len = arraysize(name);
  if (sysctlbyname("machdep.cpu.brand_string", &name, &len, NULL, 0) == 0)
    return name;
  return std::string();
}

}  // namespace base
