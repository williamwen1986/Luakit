#ifndef observer_wrapper_hpp
#define observer_wrapper_hpp

#include "base/bind.h"
#include "base/callback.h"
#include "base/tuple.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list_threadsafe.h"
#include "base/observer_list.h"
#include <memory>
#include <functional>
#include <deque>
#include <map>
#include "common/business_client_thread.h"

template <class ObserverType>
class ObserverWrapper {
public:
    void AddObserver(ObserverType* obs) {
        if (!BusinessThread::CurrentlyOn(BusinessThread::UI)) {
            LOG(WARNING) << "add observer not on ui thread. observer is " << typeid(*obs).name();
        }
        
#if defined(OS_ANDROID)
        base::AutoLock lock(list_lock_);
#endif
        observer_list_.AddObserver(obs);
    }
    
    void RemoveObserver(ObserverType* obs) {
        if (!BusinessThread::CurrentlyOn(BusinessThread::UI)) {
            LOG(WARNING) << "remove observer not on ui thread. observer is " << typeid(*obs).name();
        }
        
#if defined(OS_ANDROID)
        base::AutoLock lock(list_lock_);
#endif
        observer_list_.RemoveObserver(obs);
    }
    
    ObserverList<ObserverType>& GetObserverList() {
        return observer_list_;
    }
    
    //        FOR_EACH_OBSERVER(ConversationObserver, GetObserverList(), OnAddMembers(this, added_members));
    template<class Function, typename ... Params>
    void NotifyObserver(Function function, Params... params) {
      ObserverList<ObserverType>&  observer_list = GetObserverList();
      do {
        if ((observer_list).might_have_observers()) {                         
          typename ObserverListBase<ObserverType>::Iterator
              it_inside_observer_macro(observer_list);                        
          ObserverType* obs;    
          while (true) {
            {
#if defined(OS_ANDROID)
                base::AutoLock lock(list_lock_);
#endif
                obs = it_inside_observer_macro.GetNext();
                if (!BusinessThread::CurrentlyOn(BusinessThread::UI)) {
                    LOG(WARNING) << "notify observer not on ui thread. observer is " << typeid(*obs).name();
                }
            }
            
            if (obs != NULL) {
                DispatchToMethod(obs, function, MakeTuple(params...));
            } else {
                break;
            }
          }                                              
        }
      } while (0);
    }
private:
#if defined(OS_ANDROID)
    mutable base::Lock list_lock_;
#endif
    ObserverList<ObserverType> observer_list_;
};

#endif /* observer_wrapper_hpp */
