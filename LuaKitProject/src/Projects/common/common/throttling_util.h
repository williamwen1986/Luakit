#ifndef throttling_util_h
#define throttling_util_h

#include <functional>
#include <map>
#include <time.h>
#include <type_traits>

#include "base/bind.h"
#include "base/callback.h"
#include "base/memory/weak_ptr.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/time/time.h"
#include "base/timer/timer.h"
#include "common/base_lambda_support.h"

template<typename Key>
class ClosureThrottling {
  public:
    ClosureThrottling(time_t throttling_seconds)
      : throttling_seconds_(throttling_seconds),
        weak_factory_(this) {
    }
  
    void Do(const Key& key, const base::Closure& t) {
      auto it = throttling_map_.find(key);
      if (it == throttling_map_.end()) {
        it = throttling_map_.insert(std::make_pair(key, Info())).first;
      }
      
      auto delta = time(0) - it->second.ts_;
      if (delta > throttling_seconds_) {
        DoRun(key, t);
      } else if (!it->second.posted_) {
        it->second.posted_ = true;
        base::MessageLoopProxy::current()->PostDelayedTask(FROM_HERE, base::BindLambda(weak_factory_.GetWeakPtr(), [=]() {
            DoRun(key, t);
          }),
          base::TimeDelta::FromSeconds(throttling_seconds_)
        );
      }
    }
  
  private:
    inline void DoRun(const Key& key, const base::Closure& t) {
      throttling_map_[key].ts_ = time(0);
      throttling_map_[key].posted_ = false;
      t.Run();
    }
  
  private:
    time_t throttling_seconds_;
    struct Info {
      time_t ts_;
      bool posted_;
      
      Info() : ts_(0), posted_(false) {}
    };
    std::map<Key, Info> throttling_map_;
    base::WeakPtrFactory<ClosureThrottling> weak_factory_;
};

#endif /* throttling_util_h */
