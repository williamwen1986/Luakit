// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/files/file.h"

// TODO(rvargas): remove this (needed for kInvalidPlatformFileValue).
#include "base/platform_file.h"

namespace base {

File::Info::Info()
    : size(0),
      is_directory(false),
      is_symbolic_link(false) {
}

File::Info::~Info() {
}

File::File()
    : file_(kInvalidPlatformFileValue),
      error_details_(FILE_OK),
      created_(false),
      async_(false) {
}

#if !defined(OS_NACL)
File::File(const FilePath& name, uint32 flags)
    : file_(kInvalidPlatformFileValue),
      error_details_(FILE_OK),
      created_(false),
      async_(false) {
  Initialize(name, flags);
}
#endif

File::File(PlatformFile platform_file)
    : file_(platform_file),
      error_details_(FILE_OK),
      created_(false),
      async_(false) {
}

File::File(RValue other)
    : file_(other.object->TakePlatformFile()),
      error_details_(other.object->error_details()),
      created_(other.object->created()),
      async_(other.object->async_) {
}

File::~File() {
  Close();
}

File& File::operator=(RValue other) {
  if (this != other.object) {
    Close();
    SetPlatformFile(other.object->TakePlatformFile());
    error_details_ = other.object->error_details();
    created_ = other.object->created();
    async_ = other.object->async_;
  }
  return *this;
}

#if !defined(OS_NACL)
void File::Initialize(const FilePath& name, uint32 flags) {
  if (name.ReferencesParent()) {
    error_details_ = FILE_ERROR_ACCESS_DENIED;
    return;
  }
  InitializeUnsafe(name, flags);
}
#endif

}  // namespace base
