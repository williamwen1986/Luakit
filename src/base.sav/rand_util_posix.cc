// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/rand_util.h"

#include <errno.h>
#include <fcntl.h>
#include <random>
#include <unistd.h>
#include <time.h>

#include "base/file_util.h"
#include "base/lazy_instance.h"
#include "base/logging.h"

namespace {

// We keep the file descriptor for /dev/urandom around so we don't need to
// reopen it (which is expensive), and since we may not even be able to reopen
// it if we are later put in a sandbox. This class wraps the file descriptor so
// we can use LazyInstance to handle opening it on the first access.
class URandomFd {
 public:
  URandomFd() {
    fd_ = open("/dev/urandom", O_RDONLY);
    DCHECK_GE(fd_, 0) << "Cannot open /dev/urandom: " << errno;
  }

  ~URandomFd() {
    close(fd_);
  }

  int fd() const { return fd_; }

 private:
  int fd_;
};

base::LazyInstance<URandomFd>::Leaky g_urandom_fd = LAZY_INSTANCE_INITIALIZER;

}  // namespace

class StdRand {
  public:
    StdRand() {
      std::srand((unsigned)time(0)); // use current time as seed for random generator
    }
};

static StdRand s_std_rand;

namespace base {

// NOTE: This function must be cryptographically secure. 
uint64 RandUint64() {
  uint64 number;

  int urandom_fd = g_urandom_fd.Pointer()->fd();
  bool fd_success = ReadFromFD(urandom_fd, reinterpret_cast<char*>(&number),
                            sizeof(number));//http://crbug.com/140076
  
  bool cpp_success = false;
  if (!fd_success) {
    cpp_success = true;
    std::random_device rd;
    number = ((uint64)rd()<<32) | rd();
  }
  
  CHECK(fd_success || cpp_success);

  return number;
}

int GetUrandomFD(void) {
  return g_urandom_fd.Pointer()->fd();
}

}  // namespace base
