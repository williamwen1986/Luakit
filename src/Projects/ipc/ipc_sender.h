// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IPC_IPC_SENDER_H_
#define IPC_IPC_SENDER_H_

#include "ipc/ipc_export.h"

namespace IPC {

class Message;

class IPC_EXPORT Sender {
 public:
  // Sends the given IPC message.  The implementor takes ownership of the
  // given Message regardless of whether or not this method succeeds.  This
  // is done to make this method easier to use.  Returns true on success and
  // false otherwise.
  virtual bool Send(Message* msg) = 0;

 protected:
  virtual ~Sender() {}
};

}  // namespace IPC

#endif  // IPC_IPC_SENDER_H_
