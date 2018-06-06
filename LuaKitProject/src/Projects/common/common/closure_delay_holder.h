#ifndef delayed_then_h
#define delayed_then_h

#include "base/bind.h"
#include "base/callback.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_loop_proxy.h"

class ClosureDelayHolder : public base::RefCountedThreadSafe<ClosureDelayHolder> {
 public:
  static scoped_refptr<ClosureDelayHolder> MakeHolder() {
    return make_scoped_refptr<ClosureDelayHolder>(new ClosureDelayHolder());
  }
  
  ~ClosureDelayHolder() {
    if (!then_.is_null()) {
      if (proxy_ == base::MessageLoopProxy::current()) {
        then_.Run();
      } else {
        proxy_->PostTask(FROM_HERE, then_);
      }
    }
  }

  void SetClosure(const base::Closure& then) {
    then_ = then;
  }
  
  inline scoped_refptr<ClosureDelayHolder> MakeDelay() {
    return this;
  }

 private:
  base::Closure                           then_;
  scoped_refptr<base::MessageLoopProxy>   proxy_;

  ClosureDelayHolder() :
    proxy_(base::MessageLoopProxy::current()) {
  }

  DISALLOW_COPY_AND_ASSIGN(ClosureDelayHolder);
};


#endif /* delayed_then_h */
