// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IPC_IPC_CHANNEL_NACL_H_
#define IPC_IPC_CHANNEL_NACL_H_

#include <deque>
#include <string>

#include "base/memory/linked_ptr.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/process/process.h"
#include "base/threading/simple_thread.h"
#include "ipc/ipc_channel.h"
#include "ipc/ipc_channel_reader.h"

namespace IPC {

// Contains the results from one call to imc_recvmsg (data and file
// descriptors).
struct MessageContents;

// Similar to the Posix version of ChannelImpl but for Native Client code.
// This is somewhat different because sendmsg/recvmsg here do not follow POSIX
// semantics. Instead, they are implemented by a custom embedding of
// NaClDescCustom. See NaClIPCAdapter for the trusted-side implementation.
//
// We don't need to worry about complicated set up and READWRITE mode for
// sharing handles. We also currently do not support passing file descriptors or
// named pipes, and we use background threads to emulate signaling when we can
// read or write without blocking.
class Channel::ChannelImpl : public internal::ChannelReader {
 public:
  // Mirror methods of Channel, see ipc_channel.h for description.
  ChannelImpl(const IPC::ChannelHandle& channel_handle,
              Mode mode,
              Listener* listener);
  virtual ~ChannelImpl();

  // Channel implementation.
  bool Connect();
  void Close();
  bool Send(Message* message);

  // Posted to the main thread by ReaderThreadRunner.
  void DidRecvMsg(scoped_ptr<MessageContents> contents);
  void ReadDidFail();

 private:
  class ReaderThreadRunner;

  bool CreatePipe(const IPC::ChannelHandle& channel_handle);
  bool ProcessOutgoingMessages();

  // ChannelReader implementation.
  virtual ReadState ReadData(char* buffer,
                             int buffer_len,
                             int* bytes_read) OVERRIDE;
  virtual bool WillDispatchInputMessage(Message* msg) OVERRIDE;
  virtual bool DidEmptyInputBuffers() OVERRIDE;
  virtual void HandleInternalMessage(const Message& msg) OVERRIDE;

  Mode mode_;
  bool waiting_connect_;

  // The pipe used for communication.
  int pipe_;

  // The "name" of our pipe.  On Windows this is the global identifier for
  // the pipe.  On POSIX it's used as a key in a local map of file descriptors.
  // For NaCl, we don't actually support looking up file descriptors by name,
  // and it's only used for debug information.
  std::string pipe_name_;

  // We use a thread for reading, so that we can simply block on reading and
  // post the received data back to the main thread to be properly interleaved
  // with other tasks in the MessagePump.
  //
  // imc_recvmsg supports non-blocking reads, but there's no easy way to be
  // informed when a write or read can be done without blocking (this is handled
  // by libevent in Posix).
  scoped_ptr<ReaderThreadRunner> reader_thread_runner_;
  scoped_ptr<base::DelegateSimpleThread> reader_thread_;

  // IPC::ChannelReader expects to be able to call ReadData on us to
  // synchronously read data waiting in the pipe's buffer without blocking.
  // Since we can't do that (see 1 and 2 above), the reader thread does blocking
  // reads and posts the data over to the main thread in MessageContents. Each
  // MessageContents object is the result of one call to "imc_recvmsg".
  // DidRecvMsg breaks the MessageContents out in to the data and the file
  // descriptors, and puts them on these two queues.
  // TODO(dmichael): There's probably a more efficient way to emulate this with
  //                 a circular buffer or something, so we don't have to do so
  //                 many heap allocations. But it maybe isn't worth
  //                 the trouble given that we probably want to implement 1 and
  //                 2 above in NaCl eventually.
  // When ReadData is called, it pulls the bytes out of this queue in order.
  std::deque<linked_ptr<std::vector<char> > > read_queue_;
  // Queue of file descriptors extracted from imc_recvmsg messages.
  // NOTE: The implementation assumes underlying storage here is contiguous, so
  // don't change to something like std::deque<> without changing the
  // implementation!
  std::vector<int> input_fds_;

  // This queue is used when a message is sent prior to Connect having been
  // called. Normally after we're connected, the queue is empty.
  std::deque<linked_ptr<Message> > output_queue_;

  base::WeakPtrFactory<ChannelImpl> weak_ptr_factory_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(ChannelImpl);
};

}  // namespace IPC

#endif  // IPC_IPC_CHANNEL_NACL_H_
