#ifndef weak_task_runner_proxy_h
#define weak_task_runner_proxy_h

#include "base/callback.h"
#include "base/location.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/memory/ref_counted.h"
#include "base/threading/non_thread_safe.h"

/**
 * @class WeakTaskRunnerProxy
 * @brief 保证service的回调在service析构后不执行、callback对象在service所在线程析构
 */
class WeakTaskRunnerProxy : public base::RefCountedThreadSafe<WeakTaskRunnerProxy>, base::NonThreadSafe {
 public:
  WeakTaskRunnerProxy();
  
  void Invalid();

  bool PostTask(const tracked_objects::Location& from_here, const base::Closure& task);

 private:
  void TaskWrapper(const base::Closure& task);

 private:
  bool valid_;
  scoped_refptr<base::MessageLoopProxy> loop_proxy_;
};

#endif /* weak_task_runner_proxy_h */
