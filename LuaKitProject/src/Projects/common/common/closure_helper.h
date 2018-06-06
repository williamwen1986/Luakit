#ifndef closure_helper_h
#define closure_helper_h

#include <vector>
#include "base/callback.h"
#include "base/message_loop/message_loop_proxy.h"
#include "common/common/business_client_thread.h"


class ClosureHelper {
 public:
  static void PostToMultiThreads(const base::Closure& closure, const std::vector<BusinessThreadID>& thread_ids) {
    for (auto thread_id : thread_ids) {
      BusinessThread::PostTask(thread_id, FROM_HERE, closure);
    }
  }
};

#endif /* closure_helper_h */
