// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/sync_socket.h"

#include <stdio.h>
#include <string>
#include <sstream>

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "base/threading/thread.h"
#include "ipc/ipc_test_base.h"
#include "testing/gtest/include/gtest/gtest.h"

#if defined(OS_POSIX)
#include "base/file_descriptor_posix.h"
#endif

// IPC messages for testing ----------------------------------------------------

#define IPC_MESSAGE_IMPL
#include "ipc/ipc_message_macros.h"

#define IPC_MESSAGE_START TestMsgStart

// Message class to pass a base::SyncSocket::Handle to another process.  This
// is not as easy as it sounds, because of the differences in transferring
// Windows HANDLEs versus posix file descriptors.
#if defined(OS_WIN)
IPC_MESSAGE_CONTROL1(MsgClassSetHandle, base::SyncSocket::Handle)
#elif defined(OS_POSIX)
IPC_MESSAGE_CONTROL1(MsgClassSetHandle, base::FileDescriptor)
#endif

// Message class to pass a response to the server.
IPC_MESSAGE_CONTROL1(MsgClassResponse, std::string)

// Message class to tell the server to shut down.
IPC_MESSAGE_CONTROL0(MsgClassShutdown)

// -----------------------------------------------------------------------------

namespace {

const char kHelloString[] = "Hello, SyncSocket Client";
const size_t kHelloStringLength = arraysize(kHelloString);

// The SyncSocket server listener class processes two sorts of
// messages from the client.
class SyncSocketServerListener : public IPC::Listener {
 public:
  SyncSocketServerListener() : chan_(NULL) {
  }

  void Init(IPC::Channel* chan) {
    chan_ = chan;
  }

  virtual bool OnMessageReceived(const IPC::Message& msg) OVERRIDE {
    if (msg.routing_id() == MSG_ROUTING_CONTROL) {
      IPC_BEGIN_MESSAGE_MAP(SyncSocketServerListener, msg)
        IPC_MESSAGE_HANDLER(MsgClassSetHandle, OnMsgClassSetHandle)
        IPC_MESSAGE_HANDLER(MsgClassShutdown, OnMsgClassShutdown)
      IPC_END_MESSAGE_MAP()
    }
    return true;
  }

 private:
  // This sort of message is sent first, causing the transfer of
  // the handle for the SyncSocket.  This message sends a buffer
  // on the SyncSocket and then sends a response to the client.
#if defined(OS_WIN)
  void OnMsgClassSetHandle(const base::SyncSocket::Handle handle) {
    SetHandle(handle);
  }
#elif defined(OS_POSIX)
  void OnMsgClassSetHandle(const base::FileDescriptor& fd_struct) {
    SetHandle(fd_struct.fd);
  }
#else
# error "What platform?"
#endif  // defined(OS_WIN)

  void SetHandle(base::SyncSocket::Handle handle) {
    base::SyncSocket sync_socket(handle);
    EXPECT_EQ(sync_socket.Send(kHelloString, kHelloStringLength),
              kHelloStringLength);
    IPC::Message* msg = new MsgClassResponse(kHelloString);
    EXPECT_TRUE(chan_->Send(msg));
  }

  // When the client responds, it sends back a shutdown message,
  // which causes the message loop to exit.
  void OnMsgClassShutdown() {
    base::MessageLoop::current()->Quit();
  }

  IPC::Channel* chan_;

  DISALLOW_COPY_AND_ASSIGN(SyncSocketServerListener);
};

// Runs the fuzzing server child mode. Returns when the preset number of
// messages have been received.
MULTIPROCESS_IPC_TEST_CLIENT_MAIN(SyncSocketServerClient) {
  base::MessageLoopForIO main_message_loop;
  SyncSocketServerListener listener;
  IPC::Channel channel(IPCTestBase::GetChannelName("SyncSocketServerClient"),
                       IPC::Channel::MODE_CLIENT,
                       &listener);
  EXPECT_TRUE(channel.Connect());
  listener.Init(&channel);
  base::MessageLoop::current()->Run();
  return 0;
}

// The SyncSocket client listener only processes one sort of message,
// a response from the server.
class SyncSocketClientListener : public IPC::Listener {
 public:
  SyncSocketClientListener() {
  }

  void Init(base::SyncSocket* socket, IPC::Channel* chan) {
    socket_ = socket;
    chan_ = chan;
  }

  virtual bool OnMessageReceived(const IPC::Message& msg) OVERRIDE {
    if (msg.routing_id() == MSG_ROUTING_CONTROL) {
      IPC_BEGIN_MESSAGE_MAP(SyncSocketClientListener, msg)
        IPC_MESSAGE_HANDLER(MsgClassResponse, OnMsgClassResponse)
      IPC_END_MESSAGE_MAP()
    }
    return true;
  }

 private:
  // When a response is received from the server, it sends the same
  // string as was written on the SyncSocket.  These are compared
  // and a shutdown message is sent back to the server.
  void OnMsgClassResponse(const std::string& str) {
    // We rely on the order of sync_socket.Send() and chan_->Send() in
    // the SyncSocketServerListener object.
    EXPECT_EQ(kHelloStringLength, socket_->Peek());
    char buf[kHelloStringLength];
    socket_->Receive(static_cast<void*>(buf), kHelloStringLength);
    EXPECT_EQ(strcmp(str.c_str(), buf), 0);
    // After receiving from the socket there should be no bytes left.
    EXPECT_EQ(0U, socket_->Peek());
    IPC::Message* msg = new MsgClassShutdown();
    EXPECT_TRUE(chan_->Send(msg));
    base::MessageLoop::current()->Quit();
  }

  base::SyncSocket* socket_;
  IPC::Channel* chan_;

  DISALLOW_COPY_AND_ASSIGN(SyncSocketClientListener);
};

class SyncSocketTest : public IPCTestBase {
};

TEST_F(SyncSocketTest, SanityTest) {
  Init("SyncSocketServerClient");

  SyncSocketClientListener listener;
  CreateChannel(&listener);
  ASSERT_TRUE(StartClient());
  // Create a pair of SyncSockets.
  base::SyncSocket pair[2];
  base::SyncSocket::CreatePair(&pair[0], &pair[1]);
  // Immediately after creation there should be no pending bytes.
  EXPECT_EQ(0U, pair[0].Peek());
  EXPECT_EQ(0U, pair[1].Peek());
  base::SyncSocket::Handle target_handle;
  // Connect the channel and listener.
  ASSERT_TRUE(ConnectChannel());
  listener.Init(&pair[0], channel());
#if defined(OS_WIN)
  // On windows we need to duplicate the handle into the server process.
  BOOL retval = DuplicateHandle(GetCurrentProcess(), pair[1].handle(),
                                client_process(), &target_handle,
                                0, FALSE, DUPLICATE_SAME_ACCESS);
  EXPECT_TRUE(retval);
  // Set up a message to pass the handle to the server.
  IPC::Message* msg = new MsgClassSetHandle(target_handle);
#else
  target_handle = pair[1].handle();
  // Set up a message to pass the handle to the server.
  base::FileDescriptor filedesc(target_handle, false);
  IPC::Message* msg = new MsgClassSetHandle(filedesc);
#endif  // defined(OS_WIN)
  EXPECT_TRUE(sender()->Send(msg));
  // Use the current thread as the I/O thread.
  base::MessageLoop::current()->Run();
  // Shut down.
  pair[0].Close();
  pair[1].Close();
  EXPECT_TRUE(WaitForClientShutdown());
  DestroyChannel();
}

// A blocking read operation that will block the thread until it receives
// |length| bytes of packets or Shutdown() is called on another thread.
static void BlockingRead(base::SyncSocket* socket, char* buf,
                         size_t length, size_t* received) {
  DCHECK(buf != NULL);
  // Notify the parent thread that we're up and running.
  socket->Send(kHelloString, kHelloStringLength);
  *received = socket->Receive(buf, length);
}

// Tests that we can safely end a blocking Receive operation on one thread
// from another thread by disconnecting (but not closing) the socket.
TEST_F(SyncSocketTest, DisconnectTest) {
  base::CancelableSyncSocket pair[2];
  ASSERT_TRUE(base::CancelableSyncSocket::CreatePair(&pair[0], &pair[1]));

  base::Thread worker("BlockingThread");
  worker.Start();

  // Try to do a blocking read from one of the sockets on the worker thread.
  char buf[0xff];
  size_t received = 1U;  // Initialize to an unexpected value.
  worker.message_loop()->PostTask(FROM_HERE,
      base::Bind(&BlockingRead, &pair[0], &buf[0], arraysize(buf), &received));

  // Wait for the worker thread to say hello.
  char hello[kHelloStringLength] = {0};
  pair[1].Receive(&hello[0], sizeof(hello));
  EXPECT_EQ(0, strcmp(hello, kHelloString));
  // Give the worker a chance to start Receive().
  base::PlatformThread::YieldCurrentThread();

  // Now shut down the socket that the thread is issuing a blocking read on
  // which should cause Receive to return with an error.
  pair[0].Shutdown();

  worker.Stop();

  EXPECT_EQ(0U, received);
}

// Tests that read is a blocking operation.
TEST_F(SyncSocketTest, BlockingReceiveTest) {
  base::CancelableSyncSocket pair[2];
  ASSERT_TRUE(base::CancelableSyncSocket::CreatePair(&pair[0], &pair[1]));

  base::Thread worker("BlockingThread");
  worker.Start();

  // Try to do a blocking read from one of the sockets on the worker thread.
  char buf[kHelloStringLength] = {0};
  size_t received = 1U;  // Initialize to an unexpected value.
  worker.message_loop()->PostTask(FROM_HERE,
      base::Bind(&BlockingRead, &pair[0], &buf[0],
                 kHelloStringLength, &received));

  // Wait for the worker thread to say hello.
  char hello[kHelloStringLength] = {0};
  pair[1].Receive(&hello[0], sizeof(hello));
  EXPECT_EQ(0, strcmp(hello, kHelloString));
  // Give the worker a chance to start Receive().
  base::PlatformThread::YieldCurrentThread();

  // Send a message to the socket on the blocking thead, it should free the
  // socket from Receive().
  pair[1].Send(kHelloString, kHelloStringLength);
  worker.Stop();

  // Verify the socket has received the message.
  EXPECT_TRUE(strcmp(buf, kHelloString) == 0);
  EXPECT_EQ(kHelloStringLength, received);
}

// Tests that the write operation is non-blocking and returns immediately
// when there is insufficient space in the socket's buffer.
TEST_F(SyncSocketTest, NonBlockingWriteTest) {
  base::CancelableSyncSocket pair[2];
  ASSERT_TRUE(base::CancelableSyncSocket::CreatePair(&pair[0], &pair[1]));

  // Fill up the buffer for one of the socket, Send() should not block the
  // thread even when the buffer is full.
  while (pair[0].Send(kHelloString, kHelloStringLength) != 0) {}

  // Data should be avialble on another socket.
  size_t bytes_in_buffer = pair[1].Peek();
  EXPECT_NE(bytes_in_buffer, 0U);

  // No more data can be written to the buffer since socket has been full,
  // verify that the amount of avialble data on another socket is unchanged.
  EXPECT_EQ(0U, pair[0].Send(kHelloString, kHelloStringLength));
  EXPECT_EQ(bytes_in_buffer, pair[1].Peek());

  // Read from another socket to free some space for a new write.
  char hello[kHelloStringLength] = {0};
  pair[1].Receive(&hello[0], sizeof(hello));

  // Should be able to write more data to the buffer now.
  EXPECT_EQ(kHelloStringLength, pair[0].Send(kHelloString, kHelloStringLength));
}

}  // namespace
