// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "build/build_config.h"

#include "ipc/ipc_test_base.h"

#include "base/command_line.h"
#include "base/debug/debug_on_start_win.h"
#include "base/process/kill.h"
#include "base/threading/thread.h"
#include "base/time/time.h"
#include "ipc/ipc_descriptors.h"
#include "ipc/ipc_switches.h"

#if defined(OS_POSIX)
#include "base/posix/global_descriptors.h"
#endif

// static
std::string IPCTestBase::GetChannelName(const std::string& test_client_name) {
  DCHECK(!test_client_name.empty());
  return test_client_name + "__Channel";
}

IPCTestBase::IPCTestBase()
    : client_process_(base::kNullProcessHandle) {
}

IPCTestBase::~IPCTestBase() {
}

void IPCTestBase::SetUp() {
  MultiProcessTest::SetUp();

  // Construct a fresh IO Message loop for the duration of each test.
  DCHECK(!message_loop_.get());
  message_loop_.reset(new base::MessageLoopForIO());
}

void IPCTestBase::TearDown() {
  DCHECK(message_loop_.get());
  message_loop_.reset();
  MultiProcessTest::TearDown();
}

void IPCTestBase::Init(const std::string& test_client_name) {
  DCHECK(!test_client_name.empty());
  DCHECK(test_client_name_.empty());
  test_client_name_ = test_client_name;
}

void IPCTestBase::CreateChannel(IPC::Listener* listener) {
  return CreateChannelFromChannelHandle(GetChannelName(test_client_name_),
                                        listener);
}

bool IPCTestBase::ConnectChannel() {
  CHECK(channel_.get());
  return channel_->Connect();
}

void IPCTestBase::DestroyChannel() {
  DCHECK(channel_.get());
  channel_.reset();
}

void IPCTestBase::CreateChannelFromChannelHandle(
    const IPC::ChannelHandle& channel_handle,
    IPC::Listener* listener) {
  CHECK(!channel_.get());
  CHECK(!channel_proxy_.get());
  channel_.reset(new IPC::Channel(channel_handle,
                                  IPC::Channel::MODE_SERVER,
                                  listener));
}

void IPCTestBase::CreateChannelProxy(
    IPC::Listener* listener,
    base::SingleThreadTaskRunner* ipc_task_runner) {
  CHECK(!channel_.get());
  CHECK(!channel_proxy_.get());
  channel_proxy_.reset(new IPC::ChannelProxy(GetChannelName(test_client_name_),
                                             IPC::Channel::MODE_SERVER,
                                             listener,
                                             ipc_task_runner));
}

void IPCTestBase::DestroyChannelProxy() {
  CHECK(channel_proxy_.get());
  channel_proxy_.reset();
}

bool IPCTestBase::StartClient() {
  DCHECK(client_process_ == base::kNullProcessHandle);

  std::string test_main = test_client_name_ + "TestClientMain";
  bool debug_on_start =
      CommandLine::ForCurrentProcess()->HasSwitch(switches::kDebugChildren);

#if defined(OS_WIN)
  client_process_ = MultiProcessTest::SpawnChild(test_main, debug_on_start);
#elif defined(OS_POSIX)
  base::FileHandleMappingVector fds_to_map;
  const int ipcfd = channel_.get() ? channel_->GetClientFileDescriptor() :
                                     channel_proxy_->GetClientFileDescriptor();
  if (ipcfd > -1)
    fds_to_map.push_back(std::pair<int, int>(ipcfd,
        kPrimaryIPCChannel + base::GlobalDescriptors::kBaseDescriptor));

  client_process_ = MultiProcessTest::SpawnChild(test_main,
                                                 fds_to_map,
                                                 debug_on_start);
#endif

  return client_process_ != base::kNullProcessHandle;
}

bool IPCTestBase::WaitForClientShutdown() {
  DCHECK(client_process_ != base::kNullProcessHandle);

  bool rv = base::WaitForSingleProcess(client_process_,
                                       base::TimeDelta::FromSeconds(5));
  base::CloseProcessHandle(client_process_);
  client_process_ = base::kNullProcessHandle;
  return rv;
}
