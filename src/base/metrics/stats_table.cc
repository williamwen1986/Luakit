// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/metrics/stats_table.h"

#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/shared_memory.h"
#include "base/process/process_handle.h"
#include "base/strings/string_piece.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/platform_thread.h"
#include "base/threading/thread_local_storage.h"

#if defined(OS_POSIX)
#include "base/posix/global_descriptors.h"
#include "errno.h"
#include "ipc/ipc_descriptors.h"
#endif

namespace base {

// The StatsTable uses a shared memory segment that is laid out as follows
//
// +-------------------------------------------+
// | Version | Size | MaxCounters | MaxThreads |
// +-------------------------------------------+
// | Thread names table                        |
// +-------------------------------------------+
// | Thread TID table                          |
// +-------------------------------------------+
// | Thread PID table                          |
// +-------------------------------------------+
// | Counter names table                       |
// +-------------------------------------------+
// | Data                                      |
// +-------------------------------------------+
//
// The data layout is a grid, where the columns are the thread_ids and the
// rows are the counter_ids.
//
// If the first character of the thread_name is '\0', then that column is
// empty.
// If the first character of the counter_name is '\0', then that row is
// empty.
//
// About Locking:
// This class is designed to be both multi-thread and multi-process safe.
// Aside from initialization, this is done by partitioning the data which
// each thread uses so that no locking is required.  However, to allocate
// the rows and columns of the table to particular threads, locking is
// required.
//
// At the shared-memory level, we have a lock.  This lock protects the
// shared-memory table only, and is used when we create new counters (e.g.
// use rows) or when we register new threads (e.g. use columns).  Reading
// data from the table does not require any locking at the shared memory
// level.
//
// Each process which accesses the table will create a StatsTable object.
// The StatsTable maintains a hash table of the existing counters in the
// table for faster lookup.  Since the hash table is process specific,
// each process maintains its own cache.  We avoid complexity here by never
// de-allocating from the hash table.  (Counters are dynamically added,
// but not dynamically removed).

// In order for external viewers to be able to read our shared memory,
// we all need to use the same size ints.
COMPILE_ASSERT(sizeof(int)==4, expect_4_byte_ints);

namespace {

// An internal version in case we ever change the format of this
// file, and so that we can identify our table.
const int kTableVersion = 0x13131313;

// The name for un-named counters and threads in the table.
const char kUnknownName[] = "<unknown>";

// Calculates delta to align an offset to the size of an int
inline int AlignOffset(int offset) {
  return (sizeof(int) - (offset % sizeof(int))) % sizeof(int);
}

inline int AlignedSize(int size) {
  return size + AlignOffset(size);
}

}  // namespace

// The StatsTable::Internal maintains convenience pointers into the
// shared memory segment.  Use this class to keep the data structure
// clean and accessible.
class StatsTable::Internal {
 public:
  // Various header information contained in the memory mapped segment.
  struct TableHeader {
    int version;
    int size;
    int max_counters;
    int max_threads;
  };

  // Construct a new Internal based on expected size parameters, or
  // return NULL on failure.
  static Internal* New(const std::string& name,
                       int size,
                       int max_threads,
                       int max_counters);

  SharedMemory* shared_memory() { return shared_memory_.get(); }

  // Accessors for our header pointers
  TableHeader* table_header() const { return table_header_; }
  int version() const { return table_header_->version; }
  int size() const { return table_header_->size; }
  int max_counters() const { return table_header_->max_counters; }
  int max_threads() const { return table_header_->max_threads; }

  // Accessors for our tables
  char* thread_name(int slot_id) const {
    return &thread_names_table_[
      (slot_id-1) * (StatsTable::kMaxThreadNameLength)];
  }
  PlatformThreadId* thread_tid(int slot_id) const {
    return &(thread_tid_table_[slot_id-1]);
  }
  int* thread_pid(int slot_id) const {
    return &(thread_pid_table_[slot_id-1]);
  }
  char* counter_name(int counter_id) const {
    return &counter_names_table_[
      (counter_id-1) * (StatsTable::kMaxCounterNameLength)];
  }
  int* row(int counter_id) const {
    return &data_table_[(counter_id-1) * max_threads()];
  }

 private:
  // Constructor is private because you should use New() instead.
  explicit Internal(SharedMemory* shared_memory)
      : shared_memory_(shared_memory),
        table_header_(NULL),
        thread_names_table_(NULL),
        thread_tid_table_(NULL),
        thread_pid_table_(NULL),
        counter_names_table_(NULL),
        data_table_(NULL) {
  }

  // Create or open the SharedMemory used by the stats table.
  static SharedMemory* CreateSharedMemory(const std::string& name,
                                          int size);

  // Initializes the table on first access.  Sets header values
  // appropriately and zeroes all counters.
  void InitializeTable(void* memory, int size, int max_counters,
                       int max_threads);

  // Initializes our in-memory pointers into a pre-created StatsTable.
  void ComputeMappedPointers(void* memory);

  scoped_ptr<SharedMemory> shared_memory_;
  TableHeader* table_header_;
  char* thread_names_table_;
  PlatformThreadId* thread_tid_table_;
  int* thread_pid_table_;
  char* counter_names_table_;
  int* data_table_;

  DISALLOW_COPY_AND_ASSIGN(Internal);
};

// static
StatsTable::Internal* StatsTable::Internal::New(const std::string& name,
                                                int size,
                                                int max_threads,
                                                int max_counters) {
  scoped_ptr<SharedMemory> shared_memory(CreateSharedMemory(name, size));
  if (!shared_memory.get())
    return NULL;
  if (!shared_memory->Map(size))
    return NULL;
  void* memory = shared_memory->memory();

  scoped_ptr<Internal> internal(new Internal(shared_memory.release()));
  TableHeader* header = static_cast<TableHeader*>(memory);

  // If the version does not match, then assume the table needs
  // to be initialized.
  if (header->version != kTableVersion)
    internal->InitializeTable(memory, size, max_counters, max_threads);

  // We have a valid table, so compute our pointers.
  internal->ComputeMappedPointers(memory);

  return internal.release();
}

// static
SharedMemory* StatsTable::Internal::CreateSharedMemory(const std::string& name,
                                                       int size) {
#if defined(OS_POSIX)
  GlobalDescriptors* global_descriptors = GlobalDescriptors::GetInstance();
  if (global_descriptors->MaybeGet(kStatsTableSharedMemFd) != -1) {
    // Open the shared memory file descriptor passed by the browser process.
    FileDescriptor file_descriptor(
        global_descriptors->Get(kStatsTableSharedMemFd), false);
    return new SharedMemory(file_descriptor, false);
  }
  // Otherwise we need to create it.
  scoped_ptr<SharedMemory> shared_memory(new SharedMemory());
  if (!shared_memory->CreateAnonymous(size))
    return NULL;
  return shared_memory.release();
#elif defined(OS_WIN)
  scoped_ptr<SharedMemory> shared_memory(new SharedMemory());
  if (!shared_memory->CreateNamed(name, true, size))
    return NULL;
  return shared_memory.release();
#endif
}

void StatsTable::Internal::InitializeTable(void* memory, int size,
                                          int max_counters,
                                          int max_threads) {
  // Zero everything.
  memset(memory, 0, size);

  // Initialize the header.
  TableHeader* header = static_cast<TableHeader*>(memory);
  header->version = kTableVersion;
  header->size = size;
  header->max_counters = max_counters;
  header->max_threads = max_threads;
}

void StatsTable::Internal::ComputeMappedPointers(void* memory) {
  char* data = static_cast<char*>(memory);
  int offset = 0;

  table_header_ = reinterpret_cast<TableHeader*>(data);
  offset += sizeof(*table_header_);
  offset += AlignOffset(offset);

  // Verify we're looking at a valid StatsTable.
  DCHECK_EQ(table_header_->version, kTableVersion);

  thread_names_table_ = reinterpret_cast<char*>(data + offset);
  offset += sizeof(char) *
            max_threads() * StatsTable::kMaxThreadNameLength;
  offset += AlignOffset(offset);

  thread_tid_table_ = reinterpret_cast<PlatformThreadId*>(data + offset);
  offset += sizeof(int) * max_threads();
  offset += AlignOffset(offset);

  thread_pid_table_ = reinterpret_cast<int*>(data + offset);
  offset += sizeof(int) * max_threads();
  offset += AlignOffset(offset);

  counter_names_table_ = reinterpret_cast<char*>(data + offset);
  offset += sizeof(char) *
            max_counters() * StatsTable::kMaxCounterNameLength;
  offset += AlignOffset(offset);

  data_table_ = reinterpret_cast<int*>(data + offset);
  offset += sizeof(int) * max_threads() * max_counters();

  DCHECK_EQ(offset, size());
}

// TLSData carries the data stored in the TLS slots for the
// StatsTable.  This is used so that we can properly cleanup when the
// thread exits and return the table slot.
//
// Each thread that calls RegisterThread in the StatsTable will have
// a TLSData stored in its TLS.
struct StatsTable::TLSData {
  StatsTable* table;
  int slot;
};

// We keep a singleton table which can be easily accessed.
StatsTable* global_table = NULL;

StatsTable::StatsTable(const std::string& name, int max_threads,
                       int max_counters)
    : internal_(NULL),
      tls_index_(SlotReturnFunction) {
  int table_size =
    AlignedSize(sizeof(Internal::TableHeader)) +
    AlignedSize((max_counters * sizeof(char) * kMaxCounterNameLength)) +
    AlignedSize((max_threads * sizeof(char) * kMaxThreadNameLength)) +
    AlignedSize(max_threads * sizeof(int)) +
    AlignedSize(max_threads * sizeof(int)) +
    AlignedSize((sizeof(int) * (max_counters * max_threads)));

  internal_ = Internal::New(name, table_size, max_threads, max_counters);

  if (!internal_)
    DPLOG(ERROR) << "StatsTable did not initialize";
}

StatsTable::~StatsTable() {
  // Before we tear down our copy of the table, be sure to
  // unregister our thread.
  UnregisterThread();

  // Return ThreadLocalStorage.  At this point, if any registered threads
  // still exist, they cannot Unregister.
  tls_index_.Free();

  // Cleanup our shared memory.
  delete internal_;

  // If we are the global table, unregister ourselves.
  if (global_table == this)
    global_table = NULL;
}

StatsTable* StatsTable::current() {
  return global_table;
}

void StatsTable::set_current(StatsTable* value) {
  global_table = value;
}

int StatsTable::GetSlot() const {
  TLSData* data = GetTLSData();
  if (!data)
    return 0;
  return data->slot;
}

int StatsTable::RegisterThread(const std::string& name) {
  int slot = 0;
  if (!internal_)
    return 0;

  // Registering a thread requires that we lock the shared memory
  // so that two threads don't grab the same slot.  Fortunately,
  // thread creation shouldn't happen in inner loops.
  {
    SharedMemoryAutoLock lock(internal_->shared_memory());
    slot = FindEmptyThread();
    if (!slot) {
      return 0;
    }

    // We have space, so consume a column in the table.
    std::string thread_name = name;
    if (name.empty())
      thread_name = kUnknownName;
    strlcpy(internal_->thread_name(slot), thread_name.c_str(),
            kMaxThreadNameLength);
    *(internal_->thread_tid(slot)) = PlatformThread::CurrentId();
    *(internal_->thread_pid(slot)) = GetCurrentProcId();
  }

  // Set our thread local storage.
  TLSData* data = new TLSData;
  data->table = this;
  data->slot = slot;
  tls_index_.Set(data);
  return slot;
}

int StatsTable::CountThreadsRegistered() const {
  if (!internal_)
    return 0;

  // Loop through the shared memory and count the threads that are active.
  // We intentionally do not lock the table during the operation.
  int count = 0;
  for (int index = 1; index <= internal_->max_threads(); index++) {
    char* name = internal_->thread_name(index);
    if (*name != '\0')
      count++;
  }
  return count;
}

int StatsTable::FindCounter(const std::string& name) {
  // Note: the API returns counters numbered from 1..N, although
  // internally, the array is 0..N-1.  This is so that we can return
  // zero as "not found".
  if (!internal_)
    return 0;

  // Create a scope for our auto-lock.
  {
    AutoLock scoped_lock(counters_lock_);

    // Attempt to find the counter.
    CountersMap::const_iterator iter;
    iter = counters_.find(name);
    if (iter != counters_.end())
      return iter->second;
  }

  // Counter does not exist, so add it.
  return AddCounter(name);
}

int* StatsTable::GetLocation(int counter_id, int slot_id) const {
  if (!internal_)
    return NULL;
  if (slot_id > internal_->max_threads())
    return NULL;

  int* row = internal_->row(counter_id);
  return &(row[slot_id-1]);
}

const char* StatsTable::GetRowName(int index) const {
  if (!internal_)
    return NULL;

  return internal_->counter_name(index);
}

int StatsTable::GetRowValue(int index) const {
  return GetRowValue(index, 0);
}

int StatsTable::GetRowValue(int index, int pid) const {
  if (!internal_)
    return 0;

  int rv = 0;
  int* row = internal_->row(index);
  for (int slot_id = 1; slot_id <= internal_->max_threads(); slot_id++) {
    if (pid == 0 || *internal_->thread_pid(slot_id) == pid)
      rv += row[slot_id-1];
  }
  return rv;
}

int StatsTable::GetCounterValue(const std::string& name) {
  return GetCounterValue(name, 0);
}

int StatsTable::GetCounterValue(const std::string& name, int pid) {
  if (!internal_)
    return 0;

  int row = FindCounter(name);
  if (!row)
    return 0;
  return GetRowValue(row, pid);
}

int StatsTable::GetMaxCounters() const {
  if (!internal_)
    return 0;
  return internal_->max_counters();
}

int StatsTable::GetMaxThreads() const {
  if (!internal_)
    return 0;
  return internal_->max_threads();
}

int* StatsTable::FindLocation(const char* name) {
  // Get the static StatsTable
  StatsTable *table = StatsTable::current();
  if (!table)
    return NULL;

  // Get the slot for this thread.  Try to register
  // it if none exists.
  int slot = table->GetSlot();
  if (!slot && !(slot = table->RegisterThread(std::string())))
    return NULL;

  // Find the counter id for the counter.
  std::string str_name(name);
  int counter = table->FindCounter(str_name);

  // Now we can find the location in the table.
  return table->GetLocation(counter, slot);
}

void StatsTable::UnregisterThread() {
  UnregisterThread(GetTLSData());
}

void StatsTable::UnregisterThread(TLSData* data) {
  if (!data)
    return;
  DCHECK(internal_);

  // Mark the slot free by zeroing out the thread name.
  char* name = internal_->thread_name(data->slot);
  *name = '\0';

  // Remove the calling thread's TLS so that it cannot use the slot.
  tls_index_.Set(NULL);
  delete data;
}

void StatsTable::SlotReturnFunction(void* data) {
  // This is called by the TLS destructor, which on some platforms has
  // already cleared the TLS info, so use the tls_data argument
  // rather than trying to fetch it ourselves.
  TLSData* tls_data = static_cast<TLSData*>(data);
  if (tls_data) {
    DCHECK(tls_data->table);
    tls_data->table->UnregisterThread(tls_data);
  }
}

int StatsTable::FindEmptyThread() const {
  // Note: the API returns slots numbered from 1..N, although
  // internally, the array is 0..N-1.  This is so that we can return
  // zero as "not found".
  //
  // The reason for doing this is because the thread 'slot' is stored
  // in TLS, which is always initialized to zero, not -1.  If 0 were
  // returned as a valid slot number, it would be confused with the
  // uninitialized state.
  if (!internal_)
    return 0;

  int index = 1;
  for (; index <= internal_->max_threads(); index++) {
    char* name = internal_->thread_name(index);
    if (!*name)
      break;
  }
  if (index > internal_->max_threads())
    return 0;  // The table is full.
  return index;
}

int StatsTable::FindCounterOrEmptyRow(const std::string& name) const {
  // Note: the API returns slots numbered from 1..N, although
  // internally, the array is 0..N-1.  This is so that we can return
  // zero as "not found".
  //
  // There isn't much reason for this other than to be consistent
  // with the way we track columns for thread slots.  (See comments
  // in FindEmptyThread for why it is done this way).
  if (!internal_)
    return 0;

  int free_slot = 0;
  for (int index = 1; index <= internal_->max_counters(); index++) {
    char* row_name = internal_->counter_name(index);
    if (!*row_name && !free_slot)
      free_slot = index;  // save that we found a free slot
    else if (!strncmp(row_name, name.c_str(), kMaxCounterNameLength))
      return index;
  }
  return free_slot;
}

int StatsTable::AddCounter(const std::string& name) {
  if (!internal_)
    return 0;

  int counter_id = 0;
  {
    // To add a counter to the shared memory, we need the
    // shared memory lock.
    SharedMemoryAutoLock lock(internal_->shared_memory());

    // We have space, so create a new counter.
    counter_id = FindCounterOrEmptyRow(name);
    if (!counter_id)
      return 0;

    std::string counter_name = name;
    if (name.empty())
      counter_name = kUnknownName;
    strlcpy(internal_->counter_name(counter_id), counter_name.c_str(),
            kMaxCounterNameLength);
  }

  // now add to our in-memory cache
  {
    AutoLock lock(counters_lock_);
    counters_[name] = counter_id;
  }
  return counter_id;
}

StatsTable::TLSData* StatsTable::GetTLSData() const {
  TLSData* data =
    static_cast<TLSData*>(tls_index_.Get());
  if (!data)
    return NULL;

  DCHECK(data->slot);
  DCHECK_EQ(data->table, this);
  return data;
}

#if defined(OS_POSIX)
SharedMemoryHandle StatsTable::GetSharedMemoryHandle() const {
  if (!internal_)
    return SharedMemory::NULLHandle();
  return internal_->shared_memory()->handle();
}
#endif

}  // namespace base
