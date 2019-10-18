// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_BROWSER_THREAD_DELEGATE_H_
#define CONTENT_PUBLIC_BROWSER_BROWSER_THREAD_DELEGATE_H_
#pragma once

namespace content {

// A class with this type may be registered via
// BusinessThread::SetDelegate.
//
// If registered as such, it will receive an Init() call right before
// the BusinessThread in question starts its message loop (and right
// after the BusinessThread has done its own initialization), and a
// CleanUp call right after the message loop ends (and before the
// BusinessThread has done its own clean-up).
class BusinessThreadDelegate {
 public:
  virtual ~BusinessThreadDelegate() {}

  // Called just prior to starting the message loop.
  virtual void Init() = 0;

  // Called just after the message loop ends.
  virtual void CleanUp() = 0;
};

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_BROWSER_THREAD_DELEGATE_H_
