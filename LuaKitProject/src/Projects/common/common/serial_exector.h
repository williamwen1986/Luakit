#ifndef serial_exector_h
#define serial_exector_h

#include <map>
#include <list>

#include "base/callback.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/threading/non_thread_safe.h"

class SerialExecutorBase : public base::SupportsWeakPtr<SerialExecutorBase>,
                           public base::NonThreadSafe {
 public:
  virtual ~SerialExecutorBase() {}
};

template<typename Key>
class SerialExecutor : public SerialExecutorBase {
 public:

  typedef std::list<base::Closure> TaskList;

  struct TaskQueue {
    TaskList task_list;
    Key key;
    bool executing;

    TaskQueue(const Key& k)
      : key(k),
        executing(false) {
    }
  };

  class SerialBarrier : public base::RefCountedThreadSafe<SerialBarrier> {
   public:
    SerialBarrier(SerialExecutor* executor, TaskQueue* queue)
      : executor_(base::AsWeakPtr(executor)),
        queue_(queue),
        proxy_(base::MessageLoopProxy::current()) {
    }

    virtual ~SerialBarrier() {
      proxy_->PostTask(FROM_HERE, base::Bind(&SerialExecutor::OnFinish, executor_, base::Unretained(queue_)));
    }
    
    scoped_refptr<SerialBarrier> Barrier() {
      return this;
    }

   private:
    base::WeakPtr<SerialExecutor> executor_;
    TaskQueue* queue_;
    scoped_refptr<base::MessageLoopProxy> proxy_;
  };

  scoped_refptr<SerialBarrier> NewBarrier(const Key& key) {
    DCHECK(CalledOnValidThread());
    return new SerialBarrier(this, FindOrCreateQueue(key));
  }

  void PushTask(const Key& key, const base::Closure& task) {
    DCHECK(CalledOnValidThread());
    FindOrCreateQueue(key)->task_list.push_back(task);

    TryExecTask(key);
  }

  void TryExecTask(const Key& key) {
    DCHECK(CalledOnValidThread());
    auto queue = FindOrCreateQueue(key);
    if (queue->task_list.size() > 0 && !queue->executing) {
      // FIFO
      auto task = queue->task_list.front();
      if (!task.is_null()) {
        queue->executing = true;
        task.Run();
      }
      queue->task_list.pop_front();
    }
  }

 private:
  void OnFinish(TaskQueue* queue) {
    DCHECK(CalledOnValidThread());
    queue->executing = false;
    TryExecTask(queue->key);
  }

  TaskQueue* FindOrCreateQueue(const Key& key) {
    DCHECK(CalledOnValidThread());
    auto map_it = map_.find(key);

    if (map_it == map_.end()) {
      map_it = map_.insert(std::make_pair(key, TaskQueue(key))).first;
    }

    return &map_it->second;
  }

  typedef std::map<Key, TaskQueue> TaskListMap;

  TaskListMap map_;
};

#endif /* serial_exector_h */
