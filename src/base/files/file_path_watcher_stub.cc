// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file exists for Unix systems which don't have the inotify headers, and
// thus cannot build file_watcher_inotify.cc

#include "base/files/file_path_watcher.h"

namespace base {

namespace {

class FilePathWatcherImpl : public FilePathWatcher::PlatformDelegate {
 public:
  virtual bool Watch(const FilePath& path,
                     bool recursive,
                     const FilePathWatcher::Callback& callback) OVERRIDE {
    return false;
  }

  virtual void Cancel() OVERRIDE {}

  virtual void CancelOnMessageLoopThread() OVERRIDE {}

 protected:
  virtual ~FilePathWatcherImpl() {}
};

}  // namespace

FilePathWatcher::FilePathWatcher() {
  impl_ = new FilePathWatcherImpl();
}

}  // namespace base
