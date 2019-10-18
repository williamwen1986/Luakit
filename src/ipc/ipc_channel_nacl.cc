// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ipc/ipc_channel_nacl.h"

#include <errno.h>
#include <stddef.h>
#include <sys/types.h>

#include <algorithm>

#include "base/bind.h"
#include "base/logging.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/synchronization/lock.h"
#include "base/task_runner_util.h"
#include "base/threading/simple_thread.h"
#include "ipc/file_descriptor_set_posix.h"
#include "ipc/ipc_logging.h"
#include "native_client/src/public/imc_syscalls.h"
#include "native_client/src/public/imc_types.h"

namespace IPC {

struct MessageContents {
  std::vector<char> data;
  std::vector<int> fds;
};

namespace {

bool ReadDataOnReaderThread(int pipe, MessageContents* contents) {
  DCHECK(pipe >= 0);
  if (pipe < 0)
    return false;

  contents->data.resize(Channel::kReadBufferSize);
  contents->fds.resize(FileDescriptorSet::kMaxDescriptorsPerMessage);

  NaClAbiNaClImcMsgIoVec iov = { &contents->data[0], contents->data.size() };
  NaClAbiNaClImcMsgHdr msg = {
    &iov, 1, &contents->fds[0], contents->fds.size()
  };

  int bytes_read = imc_recvmsg(pipe, &msg, 0);

  if (bytes_read <= 0) {
    // NaClIPCAdapter::BlockingReceive returns -1 when the pipe closes (either
    // due to error or for regular shutdown).
    contents->data.clear();
    contents->fds.clear();
    return false;
  }
  DCHECK(bytes_read);
  // Resize the buffers down to the number of bytes and fds we actually read.
  contents->data.resize(bytes_read);
  contents->fds.resize(msg.desc_length);
  return true;
}

}  // namespace

class Channel::ChannelImpl::ReaderThreadRunner
    : public base::DelegateSimpleThread::Delegate {
 public:
  // |pipe|: A file descriptor from which we will read using imc_recvmsg.
  // |data_read_callback|: A callback we invoke (on the main thread) when we
  //                       have read data.
  // |failure_callback|: A callback we invoke when we have a failure reading
  //                     from |pipe|.
  // |main_message_loop|: A proxy for the main thread, where we will invoke the
  //                      above callbacks.
  ReaderThreadRunner(
      int pipe,
      base::Callback<void (scoped_ptr<MessageContents>)> data_read_callback,
      base::Callback<void ()> failure_callback,
      scoped_refptr<base::MessageLoopProxy> main_message_loop);

  // DelegateSimpleThread implementation. Reads data from the pipe in a loop
  // until either we are told to quit or a read fails.
  virtual void Run() OVERRIDE;

 private:
  int pipe_;
  base::Callback<void (scoped_ptr<MessageContents>)> data_read_callback_;
  base::Callback<void ()> failure_callback_;
  scoped_refptr<base::MessageLoopProxy> main_message_loop_;

  DISALLOW_COPY_AND_ASSIGN(ReaderThreadRunner);
};

Channel::ChannelImpl::ReaderThreadRunner::ReaderThreadRunner(
    int pipe,
    base::Callback<void (scoped_ptr<MessageContents>)> data_read_callback,
    base::Callback<void ()> failure_callback,
    scoped_refptr<base::MessageLoopProxy> main_message_loop)
    : pipe_(pipe),
      data_read_callback_(data_read_callback),
      failure_callback_(failure_callback),
      main_message_loop_(main_message_loop) {
}

void Channel::ChannelImpl::ReaderThreadRunner::Run() {
  while (true) {
    scoped_ptr<MessageContents> msg_contents(new MessageContents);
    bool success = ReadDataOnReaderThread(pipe_, msg_contents.get());
    if (success) {
      main_message_loop_->PostTask(FROM_HERE,
          base::Bind(data_read_callback_, base::Passed(&msg_contents)));
    } else {
      main_message_loop_->PostTask(FROM_HERE, failure_callback_);
      // Because the read failed, we know we're going to quit. Don't bother
      // trying to read again.
      return;
    }
  }
}

Channel::ChannelImpl::ChannelImpl(const IPC::ChannelHandle& channel_handle,
                                  Mode mode,
                                  Listener* listener)
    : ChannelReader(listener),
      mode_(mode),
      waiting_connect_(true),
      pipe_(-1),
      pipe_name_(channel_handle.name),
      weak_ptr_factory_(this) {
  if (!CreatePipe(channel_handle)) {
    // The pipe may have been closed already.
    const char *modestr = (mode_ & MODE_SERVER_FLAG) ? "server" : "client";
    LOG(WARNING) << "Unable to create pipe named \"" << channel_handle.name
                 << "\" in " << modestr << " mode";
  }
}

Channel::ChannelImpl::~ChannelImpl() {
  Close();
}

bool Channel::ChannelImpl::Connect() {
  if (pipe_ == -1) {
    DLOG(INFO) << "Channel creation failed: " << pipe_name_;
    return false;
  }

  // Note that Connect is called on the "Channel" thread (i.e., the same thread
  // where Channel::Send will be called, and the same thread that should receive
  // messages). The constructor might be invoked on another thread (see
  // ChannelProxy for an example of that). Therefore, we must wait until Connect
  // is called to decide which MessageLoopProxy to pass to ReaderThreadRunner.
  reader_thread_runner_.reset(
      new ReaderThreadRunner(
          pipe_,
          base::Bind(&Channel::ChannelImpl::DidRecvMsg,
                     weak_ptr_factory_.GetWeakPtr()),
          base::Bind(&Channel::ChannelImpl::ReadDidFail,
                     weak_ptr_factory_.GetWeakPtr()),
          base::MessageLoopProxy::current()));
  reader_thread_.reset(
      new base::DelegateSimpleThread(reader_thread_runner_.get(),
                                     "ipc_channel_nacl reader thread"));
  reader_thread_->Start();
  waiting_connect_ = false;
  // If there were any messages queued before connection, send them.
  ProcessOutgoingMessages();
  return true;
}

void Channel::ChannelImpl::Close() {
  // For now, we assume that at shutdown, the reader thread will be woken with
  // a failure (see NaClIPCAdapter::BlockingRead and CloseChannel). Or... we
  // might simply be killed with no chance to clean up anyway :-).
  // If untrusted code tries to close the channel prior to shutdown, it's likely
  // to hang.
  // TODO(dmichael): Can we do anything smarter here to make sure the reader
  //                 thread wakes up and quits?
  reader_thread_->Join();
  close(pipe_);
  pipe_ = -1;
  reader_thread_runner_.reset();
  reader_thread_.reset();
  read_queue_.clear();
  output_queue_.clear();
}

bool Channel::ChannelImpl::Send(Message* message) {
  DVLOG(2) << "sending message @" << message << " on channel @" << this
           << " with type " << message->type();
  scoped_ptr<Message> message_ptr(message);

#ifdef IPC_MESSAGE_LOG_ENABLED
  Logging::GetInstance()->OnSendMessage(message_ptr.get(), "");
#endif  // IPC_MESSAGE_LOG_ENABLED

  message->TraceMessageBegin();
  output_queue_.push_back(linked_ptr<Message>(message_ptr.release()));
  if (!waiting_connect_)
    return ProcessOutgoingMessages();

  return true;
}

void Channel::ChannelImpl::DidRecvMsg(scoped_ptr<MessageContents> contents) {
  // Close sets the pipe to -1. It's possible we'll get a buffer sent to us from
  // the reader thread after Close is called. If so, we ignore it.
  if (pipe_ == -1)
    return;

  linked_ptr<std::vector<char> > data(new std::vector<char>);
  data->swap(contents->data);
  read_queue_.push_back(data);

  input_fds_.insert(input_fds_.end(),
                    contents->fds.begin(), contents->fds.end());
  contents->fds.clear();

  // In POSIX, we would be told when there are bytes to read by implementing
  // OnFileCanReadWithoutBlocking in MessageLoopForIO::Watcher. In NaCl, we
  // instead know at this point because the reader thread posted some data to
  // us.
  ProcessIncomingMessages();
}

void Channel::ChannelImpl::ReadDidFail() {
  Close();
}

bool Channel::ChannelImpl::CreatePipe(
    const IPC::ChannelHandle& channel_handle) {
  DCHECK(pipe_ == -1);

  // There's one possible case in NaCl:
  // 1) It's a channel wrapping a pipe that is given to us.
  // We don't support these:
  // 2) It's for a named channel.
  // 3) It's for a client that we implement ourself.
  // 4) It's the initial IPC channel.

  if (channel_handle.socket.fd == -1) {
    NOTIMPLEMENTED();
    return false;
  }
  pipe_ = channel_handle.socket.fd;
  return true;
}

bool Channel::ChannelImpl::ProcessOutgoingMessages() {
  DCHECK(!waiting_connect_);  // Why are we trying to send messages if there's
                              // no connection?
  if (output_queue_.empty())
    return true;

  if (pipe_ == -1)
    return false;

  // Write out all the messages. The trusted implementation is guaranteed to not
  // block. See NaClIPCAdapter::Send for the implementation of imc_sendmsg.
  while (!output_queue_.empty()) {
    linked_ptr<Message> msg = output_queue_.front();
    output_queue_.pop_front();

    int fds[FileDescriptorSet::kMaxDescriptorsPerMessage];
    const size_t num_fds = msg->file_descriptor_set()->size();
    DCHECK(num_fds <= FileDescriptorSet::kMaxDescriptorsPerMessage);
    msg->file_descriptor_set()->GetDescriptors(fds);

    NaClAbiNaClImcMsgIoVec iov = {
      const_cast<void*>(msg->data()), msg->size()
    };
    NaClAbiNaClImcMsgHdr msgh = { &iov, 1, fds, num_fds };
    ssize_t bytes_written = imc_sendmsg(pipe_, &msgh, 0);

    DCHECK(bytes_written);  // The trusted side shouldn't return 0.
    if (bytes_written < 0) {
      // The trusted side should only ever give us an error of EPIPE. We
      // should never be interrupted, nor should we get EAGAIN.
      DCHECK(errno == EPIPE);
      Close();
      PLOG(ERROR) << "pipe_ error on "
                  << pipe_
                  << " Currently writing message of size: "
                  << msg->size();
      return false;
    } else {
      msg->file_descriptor_set()->CommitAll();
    }

    // Message sent OK!
    DVLOG(2) << "sent message @" << msg.get() << " with type " << msg->type()
             << " on fd " << pipe_;
  }
  return true;
}

Channel::ChannelImpl::ReadState Channel::ChannelImpl::ReadData(
    char* buffer,
    int buffer_len,
    int* bytes_read) {
  *bytes_read = 0;
  if (pipe_ == -1)
    return READ_FAILED;
  if (read_queue_.empty())
    return READ_PENDING;
  while (!read_queue_.empty() && *bytes_read < buffer_len) {
    linked_ptr<std::vector<char> > vec(read_queue_.front());
    size_t bytes_to_read = buffer_len - *bytes_read;
    if (vec->size() <= bytes_to_read) {
      // We can read and discard the entire vector.
      std::copy(vec->begin(), vec->end(), buffer + *bytes_read);
      *bytes_read += vec->size();
      read_queue_.pop_front();
    } else {
      // Read all the bytes we can and discard them from the front of the
      // vector. (This can be slowish, since erase has to move the back of the
      // vector to the front, but it's hopefully a temporary hack and it keeps
      // the code simple).
      std::copy(vec->begin(), vec->begin() + bytes_to_read,
                buffer + *bytes_read);
      vec->erase(vec->begin(), vec->begin() + bytes_to_read);
      *bytes_read += bytes_to_read;
    }
  }
  return READ_SUCCEEDED;
}

bool Channel::ChannelImpl::WillDispatchInputMessage(Message* msg) {
  uint16 header_fds = msg->header()->num_fds;
  CHECK(header_fds == input_fds_.size());
  if (header_fds == 0)
    return true;  // Nothing to do.

  // The shenaniganery below with &foo.front() requires input_fds_ to have
  // contiguous underlying storage (such as a simple array or a std::vector).
  // This is why the header warns not to make input_fds_ a deque<>.
  msg->file_descriptor_set()->SetDescriptors(&input_fds_.front(),
                                             header_fds);
  input_fds_.clear();
  return true;
}

bool Channel::ChannelImpl::DidEmptyInputBuffers() {
  // When the input data buffer is empty, the fds should be too.
  return input_fds_.empty();
}

void Channel::ChannelImpl::HandleInternalMessage(const Message& msg) {
  // The trusted side IPC::Channel should handle the "hello" handshake; we
  // should not receive the "Hello" message.
  NOTREACHED();
}

//------------------------------------------------------------------------------
// Channel's methods simply call through to ChannelImpl.

Channel::Channel(const IPC::ChannelHandle& channel_handle,
                 Mode mode,
                 Listener* listener)
    : channel_impl_(new ChannelImpl(channel_handle, mode, listener)) {
}

Channel::~Channel() {
  delete channel_impl_;
}

bool Channel::Connect() {
  return channel_impl_->Connect();
}

void Channel::Close() {
  channel_impl_->Close();
}

base::ProcessId Channel::peer_pid() const {
  // This shouldn't actually get used in the untrusted side of the proxy, and we
  // don't have the real pid anyway.
  return -1;
}

bool Channel::Send(Message* message) {
  return channel_impl_->Send(message);
}

}  // namespace IPC
