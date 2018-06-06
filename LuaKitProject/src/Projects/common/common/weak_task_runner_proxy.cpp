#include "common/weak_task_runner_proxy.h"

#include "base/bind.h"

WeakTaskRunnerProxy::WeakTaskRunnerProxy()
  : valid_(true),
    loop_proxy_(base::MessageLoopProxy::current()) {
}

void WeakTaskRunnerProxy::Invalid() {
  DCHECK(valid_);
  DCHECK(CalledOnValidThread());

  valid_ = false;
}

void WeakTaskRunnerProxy::TaskWrapper(const base::Closure& task) {
  DCHECK(CalledOnValidThread());
  if (valid_) {
    task.Run();
  } else {
    DLOG(INFO) << "task invalid by proxy";
  }
}

bool WeakTaskRunnerProxy::PostTask(const tracked_objects::Location& from_here, const base::Closure& task) {
  return loop_proxy_->PostTask(from_here, base::Bind(&WeakTaskRunnerProxy::TaskWrapper, this, task));
}
