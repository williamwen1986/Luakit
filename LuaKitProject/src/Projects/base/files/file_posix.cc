// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/files/file.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/metrics/sparse_histogram.h"
// TODO(rvargas): remove this (needed for kInvalidPlatformFileValue).
#include "base/platform_file.h"
#include "base/posix/eintr_wrapper.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/thread_restrictions.h"

#if defined(OS_ANDROID)
#include "base/os_compat_android.h"
#endif

namespace base {

// Make sure our Whence mappings match the system headers.
COMPILE_ASSERT(File::FROM_BEGIN   == SEEK_SET &&
               File::FROM_CURRENT == SEEK_CUR &&
               File::FROM_END     == SEEK_END, whence_matches_system);

namespace {

#if defined(OS_BSD) || defined(OS_MACOSX) || defined(OS_NACL)
typedef struct stat stat_wrapper_t;
static int CallFstat(int fd, stat_wrapper_t *sb) {
  base::ThreadRestrictions::AssertIOAllowed();
  return fstat(fd, sb);
}
#else
typedef struct stat64 stat_wrapper_t;
static int CallFstat(int fd, stat_wrapper_t *sb) {
  base::ThreadRestrictions::AssertIOAllowed();
  return fstat64(fd, sb);
}
#endif

// NaCl doesn't provide the following system calls, so either simulate them or
// wrap them in order to minimize the number of #ifdef's in this file.
#if !defined(OS_NACL)
static bool IsOpenAppend(PlatformFile file) {
  return (fcntl(file, F_GETFL) & O_APPEND) != 0;
}

static int CallFtruncate(PlatformFile file, int64 length) {
  return HANDLE_EINTR(ftruncate(file, length));
}

static int CallFsync(PlatformFile file) {
  return HANDLE_EINTR(fsync(file));
}

static int CallFutimes(PlatformFile file, const struct timeval times[2]) {
#ifdef __USE_XOPEN2K8
  // futimens should be available, but futimes might not be
  // http://pubs.opengroup.org/onlinepubs/9699919799/

  timespec ts_times[2];
  ts_times[0].tv_sec  = times[0].tv_sec;
  ts_times[0].tv_nsec = times[0].tv_usec * 1000;
  ts_times[1].tv_sec  = times[1].tv_sec;
  ts_times[1].tv_nsec = times[1].tv_usec * 1000;

  return futimens(file, ts_times);
#else
  return futimes(file, times);
#endif
}

static File::Error CallFctnlFlock(PlatformFile file, bool do_lock) {
  struct flock lock;
  lock.l_type = F_WRLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = 0;  // Lock entire file.
  if (HANDLE_EINTR(fcntl(file, do_lock ? F_SETLK : F_UNLCK, &lock)) == -1)
    return File::OSErrorToFileError(errno);
  return File::FILE_OK;
}
#else  // defined(OS_NACL)

static bool IsOpenAppend(PlatformFile file) {
  // NaCl doesn't implement fcntl. Since NaCl's write conforms to the POSIX
  // standard and always appends if the file is opened with O_APPEND, just
  // return false here.
  return false;
}

static int CallFtruncate(PlatformFile file, int64 length) {
  NOTIMPLEMENTED();  // NaCl doesn't implement ftruncate.
  return 0;
}

static int CallFsync(PlatformFile file) {
  NOTIMPLEMENTED();  // NaCl doesn't implement fsync.
  return 0;
}

static int CallFutimes(PlatformFile file, const struct timeval times[2]) {
  NOTIMPLEMENTED();  // NaCl doesn't implement futimes.
  return 0;
}

static File::Error CallFctnlFlock(PlatformFile file, bool do_lock) {
  NOTIMPLEMENTED();  // NaCl doesn't implement flock struct.
  return File::FILE_ERROR_INVALID_OPERATION;
}
#endif  // defined(OS_NACL)

}  // namespace

// NaCl doesn't implement system calls to open files directly.
#if !defined(OS_NACL)
// TODO(erikkay): does it make sense to support FLAG_EXCLUSIVE_* here?
void File::InitializeUnsafe(const FilePath& name, uint32 flags) {
  base::ThreadRestrictions::AssertIOAllowed();
  DCHECK(!IsValid());
  DCHECK(!(flags & FLAG_ASYNC));

  int open_flags = 0;
  if (flags & FLAG_CREATE)
    open_flags = O_CREAT | O_EXCL;

  created_ = false;

  if (flags & FLAG_CREATE_ALWAYS) {
    DCHECK(!open_flags);
    open_flags = O_CREAT | O_TRUNC;
  }

  if (flags & FLAG_OPEN_TRUNCATED) {
    DCHECK(!open_flags);
    DCHECK(flags & FLAG_WRITE);
    open_flags = O_TRUNC;
  }

  if (!open_flags && !(flags & FLAG_OPEN) && !(flags & FLAG_OPEN_ALWAYS)) {
    NOTREACHED();
    errno = EOPNOTSUPP;
    error_details_ = FILE_ERROR_FAILED;
    return;
  }

  if (flags & FLAG_WRITE && flags & FLAG_READ) {
    open_flags |= O_RDWR;
  } else if (flags & FLAG_WRITE) {
    open_flags |= O_WRONLY;
  } else if (!(flags & FLAG_READ) &&
             !(flags & FLAG_WRITE_ATTRIBUTES) &&
             !(flags & FLAG_APPEND) &&
             !(flags & FLAG_OPEN_ALWAYS)) {
    NOTREACHED();
  }

  if (flags & FLAG_TERMINAL_DEVICE)
    open_flags |= O_NOCTTY | O_NDELAY;

  if (flags & FLAG_APPEND && flags & FLAG_READ)
    open_flags |= O_APPEND | O_RDWR;
  else if (flags & FLAG_APPEND)
    open_flags |= O_APPEND | O_WRONLY;

  COMPILE_ASSERT(O_RDONLY == 0, O_RDONLY_must_equal_zero);

  int mode = S_IRUSR | S_IWUSR;
#if defined(OS_CHROMEOS)
  mode |= S_IRGRP | S_IROTH;
#endif

  int descriptor = HANDLE_EINTR(open(name.value().c_str(), open_flags, mode));

  if (flags & FLAG_OPEN_ALWAYS) {
    if (descriptor < 0) {
      open_flags |= O_CREAT;
      if (flags & FLAG_EXCLUSIVE_READ || flags & FLAG_EXCLUSIVE_WRITE)
        open_flags |= O_EXCL;   // together with O_CREAT implies O_NOFOLLOW

      descriptor = HANDLE_EINTR(open(name.value().c_str(), open_flags, mode));
      if (descriptor >= 0)
        created_ = true;
    }
  }

  if (descriptor >= 0 && (flags & (FLAG_CREATE_ALWAYS | FLAG_CREATE)))
    created_ = true;

  if ((descriptor >= 0) && (flags & FLAG_DELETE_ON_CLOSE))
    unlink(name.value().c_str());

  if (descriptor >= 0)
    error_details_ = FILE_OK;
  else
    error_details_ = File::OSErrorToFileError(errno);

  file_ = descriptor;
}
#endif  // !defined(OS_NACL)

bool File::IsValid() const {
  return file_ >= 0;
}

PlatformFile File::TakePlatformFile() {
  PlatformFile file = file_;
  file_ = kInvalidPlatformFileValue;
  return file;
}

void File::Close() {
  base::ThreadRestrictions::AssertIOAllowed();
  if (!IsValid())
    return;

  if (!IGNORE_EINTR(close(file_)))
    file_ = kInvalidPlatformFileValue;
}

int64 File::Seek(Whence whence, int64 offset) {
  base::ThreadRestrictions::AssertIOAllowed();
  DCHECK(IsValid());
  if (file_ < 0 || offset < 0)
    return -1;

  return lseek(file_, static_cast<off_t>(offset), static_cast<int>(whence));
}

int File::Read(int64 offset, char* data, int size) {
  base::ThreadRestrictions::AssertIOAllowed();
  DCHECK(IsValid());
  if (size < 0)
    return -1;

  int bytes_read = 0;
  int rv;
  do {
    rv = HANDLE_EINTR(pread(file_, data + bytes_read,
                            size - bytes_read, offset + bytes_read));
    if (rv <= 0)
      break;

    bytes_read += rv;
  } while (bytes_read < size);

  return bytes_read ? bytes_read : rv;
}

int File::ReadAtCurrentPos(char* data, int size) {
  base::ThreadRestrictions::AssertIOAllowed();
  DCHECK(IsValid());
  if (size < 0)
    return -1;

  int bytes_read = 0;
  int rv;
  do {
    rv = HANDLE_EINTR(read(file_, data, size));
    if (rv <= 0)
      break;

    bytes_read += rv;
  } while (bytes_read < size);

  return bytes_read ? bytes_read : rv;
}

int File::ReadNoBestEffort(int64 offset, char* data, int size) {
  base::ThreadRestrictions::AssertIOAllowed();
  DCHECK(IsValid());

  return HANDLE_EINTR(pread(file_, data, size, offset));
}

int File::ReadAtCurrentPosNoBestEffort(char* data, int size) {
  base::ThreadRestrictions::AssertIOAllowed();
  DCHECK(IsValid());
  if (size < 0)
    return -1;

  return HANDLE_EINTR(read(file_, data, size));
}

int File::Write(int64 offset, const char* data, int size) {
  base::ThreadRestrictions::AssertIOAllowed();

  if (IsOpenAppend(file_))
    return WriteAtCurrentPos(data, size);

  DCHECK(IsValid());
  if (size < 0)
    return -1;

  int bytes_written = 0;
  int rv;
  do {
    rv = HANDLE_EINTR(pwrite(file_, data + bytes_written,
                             size - bytes_written, offset + bytes_written));
    if (rv <= 0)
      break;

    bytes_written += rv;
  } while (bytes_written < size);

  return bytes_written ? bytes_written : rv;
}

int File::WriteAtCurrentPos(const char* data, int size) {
  base::ThreadRestrictions::AssertIOAllowed();
  DCHECK(IsValid());
  if (size < 0)
    return -1;

  int bytes_written = 0;
  int rv;
  do {
    rv = HANDLE_EINTR(write(file_, data, size));
    if (rv <= 0)
      break;

    bytes_written += rv;
  } while (bytes_written < size);

  return bytes_written ? bytes_written : rv;
}

int File::WriteAtCurrentPosNoBestEffort(const char* data, int size) {
  base::ThreadRestrictions::AssertIOAllowed();
  DCHECK(IsValid());
  if (size < 0)
    return -1;

  return HANDLE_EINTR(write(file_, data, size));
}

int64 File::GetLength() {
  DCHECK(IsValid());

  stat_wrapper_t file_info;
  if (CallFstat(file_, &file_info))
    return false;

  return file_info.st_size;
}

bool File::SetLength(int64 length) {
  base::ThreadRestrictions::AssertIOAllowed();
  DCHECK(IsValid());
  return !CallFtruncate(file_, length);
}

bool File::Flush() {
  base::ThreadRestrictions::AssertIOAllowed();
  DCHECK(IsValid());
  return !CallFsync(file_);
}

bool File::SetTimes(Time last_access_time, Time last_modified_time) {
  base::ThreadRestrictions::AssertIOAllowed();
  DCHECK(IsValid());

  timeval times[2];
  times[0] = last_access_time.ToTimeVal();
  times[1] = last_modified_time.ToTimeVal();

  return !CallFutimes(file_, times);
}

bool File::GetInfo(Info* info) {
  DCHECK(IsValid());

  stat_wrapper_t file_info;
  if (CallFstat(file_, &file_info))
    return false;

  info->is_directory = S_ISDIR(file_info.st_mode);
  info->is_symbolic_link = S_ISLNK(file_info.st_mode);
  info->size = file_info.st_size;

#if defined(OS_LINUX)
  const time_t last_modified_sec = file_info.st_mtim.tv_sec;
  const int64 last_modified_nsec = file_info.st_mtim.tv_nsec;
  const time_t last_accessed_sec = file_info.st_atim.tv_sec;
  const int64 last_accessed_nsec = file_info.st_atim.tv_nsec;
  const time_t creation_time_sec = file_info.st_ctim.tv_sec;
  const int64 creation_time_nsec = file_info.st_ctim.tv_nsec;
#elif defined(OS_ANDROID)
  const time_t last_modified_sec = file_info.st_mtime;
  const int64 last_modified_nsec = file_info.st_mtime_nsec;
  const time_t last_accessed_sec = file_info.st_atime;
  const int64 last_accessed_nsec = file_info.st_atime_nsec;
  const time_t creation_time_sec = file_info.st_ctime;
  const int64 creation_time_nsec = file_info.st_ctime_nsec;
#elif defined(OS_MACOSX) || defined(OS_IOS) || defined(OS_BSD)
  const time_t last_modified_sec = file_info.st_mtimespec.tv_sec;
  const int64 last_modified_nsec = file_info.st_mtimespec.tv_nsec;
  const time_t last_accessed_sec = file_info.st_atimespec.tv_sec;
  const int64 last_accessed_nsec = file_info.st_atimespec.tv_nsec;
  const time_t creation_time_sec = file_info.st_ctimespec.tv_sec;
  const int64 creation_time_nsec = file_info.st_ctimespec.tv_nsec;
#else
  // TODO(gavinp): Investigate a good high resolution option for OS_NACL.
  const time_t last_modified_sec = file_info.st_mtime;
  const int64 last_modified_nsec = 0;
  const time_t last_accessed_sec = file_info.st_atime;
  const int64 last_accessed_nsec = 0;
  const time_t creation_time_sec = file_info.st_ctime;
  const int64 creation_time_nsec = 0;
#endif

  info->last_modified =
      base::Time::FromTimeT(last_modified_sec) +
      base::TimeDelta::FromMicroseconds(last_modified_nsec /
                                        base::Time::kNanosecondsPerMicrosecond);
  info->last_accessed =
      base::Time::FromTimeT(last_accessed_sec) +
      base::TimeDelta::FromMicroseconds(last_accessed_nsec /
                                        base::Time::kNanosecondsPerMicrosecond);
  info->creation_time =
      base::Time::FromTimeT(creation_time_sec) +
      base::TimeDelta::FromMicroseconds(creation_time_nsec /
                                        base::Time::kNanosecondsPerMicrosecond);
  return true;
}

File::Error File::Lock() {
  return CallFctnlFlock(file_, true);
}

File::Error File::Unlock() {
  return CallFctnlFlock(file_, false);
}

// Static.
File::Error File::OSErrorToFileError(int saved_errno) {
  switch (saved_errno) {
    case EACCES:
    case EISDIR:
    case EROFS:
    case EPERM:
      return FILE_ERROR_ACCESS_DENIED;
#if !defined(OS_NACL)  // ETXTBSY not defined by NaCl.
    case ETXTBSY:
      return FILE_ERROR_IN_USE;
#endif
    case EEXIST:
      return FILE_ERROR_EXISTS;
    case ENOENT:
      return FILE_ERROR_NOT_FOUND;
    case EMFILE:
      return FILE_ERROR_TOO_MANY_OPENED;
    case ENOMEM:
      return FILE_ERROR_NO_MEMORY;
    case ENOSPC:
      return FILE_ERROR_NO_SPACE;
    case ENOTDIR:
      return FILE_ERROR_NOT_A_DIRECTORY;
    default:
#if !defined(OS_NACL)  // NaCl build has no metrics code.
      UMA_HISTOGRAM_SPARSE_SLOWLY("PlatformFile.UnknownErrors.Posix",
                                  saved_errno);
#endif
      return FILE_ERROR_FAILED;
  }
}

void File::SetPlatformFile(PlatformFile file) {
  DCHECK_EQ(file_, kInvalidPlatformFileValue);
  file_ = file;
}

}  // namespace base
