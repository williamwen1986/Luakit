// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IPC_IPC_CHANNEL_H_
#define IPC_IPC_CHANNEL_H_

#include <string>

#if defined(OS_POSIX)
#include <sys/types.h>
#endif

#include "base/compiler_specific.h"
#include "base/process/process.h"
#include "ipc/ipc_channel_handle.h"
#include "ipc/ipc_message.h"
#include "ipc/ipc_sender.h"

namespace IPC {

class Listener;

//------------------------------------------------------------------------------
// See
// http://www.chromium.org/developers/design-documents/inter-process-communication
// for overview of IPC in Chromium.

// Channels are implemented using named pipes on Windows, and
// socket pairs (or in some special cases unix domain sockets) on POSIX.
// On Windows we access pipes in various processes by name.
// On POSIX we pass file descriptors to child processes and assign names to them
// in a lookup table.
// In general on POSIX we do not use unix domain sockets due to security
// concerns and the fact that they can leave garbage around the file system
// (MacOS does not support abstract named unix domain sockets).
// You can use unix domain sockets if you like on POSIX by constructing the
// the channel with the mode set to one of the NAMED modes. NAMED modes are
// currently used by automation and service processes.

class IPC_EXPORT Channel : public Sender {
  // Security tests need access to the pipe handle.
  friend class ChannelTest;

 public:
  // Flags to test modes
  enum ModeFlags {
    MODE_NO_FLAG = 0x0,
    MODE_SERVER_FLAG = 0x1,
    MODE_CLIENT_FLAG = 0x2,
    MODE_NAMED_FLAG = 0x4,
#if defined(OS_POSIX)
    MODE_OPEN_ACCESS_FLAG = 0x8, // Don't restrict access based on client UID.
#endif
  };

  // Some Standard Modes
  enum Mode {
    MODE_NONE = MODE_NO_FLAG,
    MODE_SERVER = MODE_SERVER_FLAG,
    MODE_CLIENT = MODE_CLIENT_FLAG,
    // Channels on Windows are named by default and accessible from other
    // processes. On POSIX channels are anonymous by default and not accessible
    // from other processes. Named channels work via named unix domain sockets.
    // On Windows MODE_NAMED_SERVER is equivalent to MODE_SERVER and
    // MODE_NAMED_CLIENT is equivalent to MODE_CLIENT.
    MODE_NAMED_SERVER = MODE_SERVER_FLAG | MODE_NAMED_FLAG,
    MODE_NAMED_CLIENT = MODE_CLIENT_FLAG | MODE_NAMED_FLAG,
#if defined(OS_POSIX)
    // An "open" named server accepts connections from ANY client.
    // The caller must then implement their own access-control based on the
    // client process' user Id.
    MODE_OPEN_NAMED_SERVER = MODE_OPEN_ACCESS_FLAG | MODE_SERVER_FLAG |
                             MODE_NAMED_FLAG
#endif
  };

  // Messages internal to the IPC implementation are defined here.
  // Uses Maximum value of message type (uint16), to avoid conflicting
  // with normal message types, which are enumeration constants starting from 0.
  enum {
    // The Hello message is sent by the peer when the channel is connected.
    // The message contains just the process id (pid).
    // The message has a special routing_id (MSG_ROUTING_NONE)
    // and type (HELLO_MESSAGE_TYPE).
    HELLO_MESSAGE_TYPE = kuint16max,
    // The CLOSE_FD_MESSAGE_TYPE is used in the IPC class to
    // work around a bug in sendmsg() on Mac. When an FD is sent
    // over the socket, a CLOSE_FD_MESSAGE is sent with hops = 2.
    // The client will return the message with hops = 1, *after* it
    // has received the message that contains the FD. When we
    // receive it again on the sender side, we close the FD.
    CLOSE_FD_MESSAGE_TYPE = HELLO_MESSAGE_TYPE - 1
  };

  // The maximum message size in bytes. Attempting to receive a message of this
  // size or bigger results in a channel error.
  static const size_t kMaximumMessageSize = 128 * 1024 * 1024;

  // Amount of data to read at once from the pipe.
  static const size_t kReadBufferSize = 4 * 1024;

  // Initialize a Channel.
  //
  // |channel_handle| identifies the communication Channel. For POSIX, if
  // the file descriptor in the channel handle is != -1, the channel takes
  // ownership of the file descriptor and will close it appropriately, otherwise
  // it will create a new descriptor internally.
  // |mode| specifies whether this Channel is to operate in server mode or
  // client mode.  In server mode, the Channel is responsible for setting up the
  // IPC object, whereas in client mode, the Channel merely connects to the
  // already established IPC object.
  // |listener| receives a callback on the current thread for each newly
  // received message.
  //
  Channel(const IPC::ChannelHandle &channel_handle, Mode mode,
          Listener* listener);

  virtual ~Channel();

  // Connect the pipe.  On the server side, this will initiate
  // waiting for connections.  On the client, it attempts to
  // connect to a pre-existing pipe.  Note, calling Connect()
  // will not block the calling thread and may complete
  // asynchronously.
  bool Connect() WARN_UNUSED_RESULT;

  // Close this Channel explicitly.  May be called multiple times.
  // On POSIX calling close on an IPC channel that listens for connections will
  // cause it to close any accepted connections, and it will stop listening for
  // new connections. If you just want to close the currently accepted
  // connection and listen for new ones, use ResetToAcceptingConnectionState.
  void Close();

  // Get the process ID for the connected peer.
  //
  // Returns base::kNullProcessId if the peer is not connected yet. Watch out
  // for race conditions. You can easily get a channel to another process, but
  // if your process has not yet processed the "hello" message from the remote
  // side, this will fail. You should either make sure calling this is either
  // in response to a message from the remote side (which guarantees that it's
  // been connected), or you wait for the "connected" notification on the
  // listener.
  base::ProcessId peer_pid() const;

  // Send a message over the Channel to the listener on the other end.
  //
  // |message| must be allocated using operator new.  This object will be
  // deleted once the contents of the Message have been sent.
  virtual bool Send(Message* message) OVERRIDE;

#if defined(OS_POSIX)
  // On POSIX an IPC::Channel wraps a socketpair(), this method returns the
  // FD # for the client end of the socket.
  // This method may only be called on the server side of a channel.
  // This method can be called on any thread.
  int GetClientFileDescriptor() const;

  // Same as GetClientFileDescriptor, but transfers the ownership of the
  // file descriptor to the caller.
  // This method can be called on any thread.
  int TakeClientFileDescriptor();

  // On POSIX an IPC::Channel can either wrap an established socket, or it
  // can wrap a socket that is listening for connections. Currently an
  // IPC::Channel that listens for connections can only accept one connection
  // at a time.

  // Returns true if the channel supports listening for connections.
  bool AcceptsConnections() const;

  // Returns true if the channel supports listening for connections and is
  // currently connected.
  bool HasAcceptedConnection() const;

  // Returns true if the peer process' effective user id can be determined, in
  // which case the supplied peer_euid is updated with it.
  bool GetPeerEuid(uid_t* peer_euid) const;

  // Closes any currently connected socket, and returns to a listening state
  // for more connections.
  void ResetToAcceptingConnectionState();
#endif  // defined(OS_POSIX) && !defined(OS_NACL)

  // Returns true if a named server channel is initialized on the given channel
  // ID. Even if true, the server may have already accepted a connection.
  static bool IsNamedServerInitialized(const std::string& channel_id);

#if !defined(OS_NACL)
  // Generates a channel ID that's non-predictable and unique.
  static std::string GenerateUniqueRandomChannelID();

  // Generates a channel ID that, if passed to the client as a shared secret,
  // will validate that the client's authenticity. On platforms that do not
  // require additional this is simply calls GenerateUniqueRandomChannelID().
  // For portability the prefix should not include the \ character.
  static std::string GenerateVerifiedChannelID(const std::string& prefix);
#endif

#if defined(OS_LINUX)
  // Sandboxed processes live in a PID namespace, so when sending the IPC hello
  // message from client to server we need to send the PID from the global
  // PID namespace.
  static void SetGlobalPid(int pid);
#endif

 protected:
  // Used in Chrome by the TestSink to provide a dummy channel implementation
  // for testing. TestSink overrides the "interesting" functions in Channel so
  // no actual implementation is needed. This will cause un-overridden calls to
  // segfault. Do not use outside of test code!
  Channel() : channel_impl_(0) { }

 private:
  // PIMPL to which all channel calls are delegated.
  class ChannelImpl;
  ChannelImpl *channel_impl_;
};

}  // namespace IPC

#endif  // IPC_IPC_CHANNEL_H_
