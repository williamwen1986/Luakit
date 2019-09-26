// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TEST_TEST_FILE_UTIL_H_
#define BASE_TEST_TEST_FILE_UTIL_H_

// File utility functions used only by tests.

#include <string>

#include "base/compiler_specific.h"
#include "base/files/file_path.h"

#if defined(OS_ANDROID)
#include <jni.h>
#include "base/basictypes.h"
#endif

namespace base {

class FilePath;

// Clear a specific file from the system cache like EvictFileFromSystemCache,
// but on failure it will sleep and retry. On the Windows buildbots, eviction
// can fail if the file is marked in use, and this will throw off timings that
// rely on uncached files.
bool EvictFileFromSystemCacheWithRetry(const FilePath& file);

}  // namespace base

// TODO(brettw) move all of this to the base namespace.
namespace file_util {

// Wrapper over base::Delete. On Windows repeatedly invokes Delete in case
// of failure to workaround Windows file locking semantics. Returns true on
// success.
bool DieFileDie(const base::FilePath& file, bool recurse);

// Clear a specific file from the system cache. After this call, trying
// to access this file will result in a cold load from the hard drive.
bool EvictFileFromSystemCache(const base::FilePath& file);

#if defined(OS_WIN)
// Returns true if the volume supports Alternate Data Streams.
bool VolumeSupportsADS(const base::FilePath& path);

// Returns true if the ZoneIdentifier is correctly set to "Internet" (3).
// Note that this function must be called from the same process as
// the one that set the zone identifier.  I.e. don't use it in UI/automation
// based tests.
bool HasInternetZoneIdentifier(const base::FilePath& full_path);
#endif  // defined(OS_WIN)

// In general it's not reliable to convert a FilePath to a wstring and we use
// string16 elsewhere for Unicode strings, but in tests it is frequently
// convenient to be able to compare paths to literals like L"foobar".
std::wstring FilePathAsWString(const base::FilePath& path);
base::FilePath WStringAsFilePath(const std::wstring& path);

// For testing, make the file unreadable or unwritable.
// In POSIX, this does not apply to the root user.
bool MakeFileUnreadable(const base::FilePath& path) WARN_UNUSED_RESULT;
bool MakeFileUnwritable(const base::FilePath& path) WARN_UNUSED_RESULT;

#if defined(OS_ANDROID)
// Register the ContentUriTestUrils JNI bindings.
bool RegisterContentUriTestUtils(JNIEnv* env);

// Insert an image file into the MediaStore, and retrieve the content URI for
// testing purpose.
base::FilePath InsertImageIntoMediaStore(const base::FilePath& path);
#endif  // defined(OS_ANDROID)

// Saves the current permissions for a path, and restores it on destruction.
class PermissionRestorer {
 public:
  explicit PermissionRestorer(const base::FilePath& path);
  ~PermissionRestorer();

 private:
  const base::FilePath path_;
  void* info_;  // The opaque stored permission information.
  size_t length_;  // The length of the stored permission information.

  DISALLOW_COPY_AND_ASSIGN(PermissionRestorer);
};

}  // namespace file_util

#endif  // BASE_TEST_TEST_FILE_UTIL_H_
