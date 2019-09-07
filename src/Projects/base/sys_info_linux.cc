// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/sys_info.h"

#include <limits>

#include "base/file_util.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/sys_info_internal.h"

namespace {

int64 AmountOfMemory(int pages_name) {
  long pages = sysconf(pages_name);
  long page_size = sysconf(_SC_PAGESIZE);
  if (pages == -1 || page_size == -1) {
    NOTREACHED();
    return 0;
  }
  return static_cast<int64>(pages) * page_size;
}

int64 AmountOfPhysicalMemory() {
  return AmountOfMemory(_SC_PHYS_PAGES);
}

size_t MaxSharedMemorySize() {
  std::string contents;
  base::ReadFileToString(base::FilePath("/proc/sys/kernel/shmmax"), &contents);
  DCHECK(!contents.empty());
  if (!contents.empty() && contents[contents.length() - 1] == '\n') {
    contents.erase(contents.length() - 1);
  }

  int64 limit;
  if (!base::StringToInt64(contents, &limit)) {
    limit = 0;
  }
  if (limit < 0 ||
      static_cast<uint64>(limit) > std::numeric_limits<size_t>::max()) {
    limit = 0;
  }
  DCHECK(limit > 0);
  return static_cast<size_t>(limit);
}

base::LazyInstance<
    base::internal::LazySysInfoValue<int64, AmountOfPhysicalMemory> >::Leaky
    g_lazy_physical_memory = LAZY_INSTANCE_INITIALIZER;
base::LazyInstance<
    base::internal::LazySysInfoValue<size_t, MaxSharedMemorySize> >::Leaky
    g_lazy_max_shared_memory = LAZY_INSTANCE_INITIALIZER;

}  // namespace

namespace base {

// static
int64 SysInfo::AmountOfAvailablePhysicalMemory() {
  return AmountOfMemory(_SC_AVPHYS_PAGES);
}

// static
int64 SysInfo::AmountOfPhysicalMemory() {
  return g_lazy_physical_memory.Get().value();
}

// static
size_t SysInfo::MaxSharedMemorySize() {
  return g_lazy_max_shared_memory.Get().value();
}

// static
std::string SysInfo::CPUModelName() {
#if defined(OS_CHROMEOS) && defined(ARCH_CPU_ARMEL)
  const char kCpuModelPrefix[] = "Hardware";
#else
  const char kCpuModelPrefix[] = "model name";
#endif
  std::string contents;
  ReadFileToString(FilePath("/proc/cpuinfo"), &contents);
  DCHECK(!contents.empty());
  if (!contents.empty()) {
    std::istringstream iss(contents);
    std::string line;
    while (std::getline(iss, line)) {
      if (line.compare(0, strlen(kCpuModelPrefix), kCpuModelPrefix) == 0) {
        size_t pos = line.find(": ");
        return line.substr(pos + 2);
      }
    }
  }
  return std::string();
}

}  // namespace base
