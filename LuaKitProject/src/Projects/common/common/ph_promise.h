#ifndef ph_apromise_h
#define ph_apromise_h

#include <future>
#include <tuple>

#include "base/callback.h"

namespace ph {

template <typename... Args>
class Future {
protected:
    typedef Future<Args...> this_type;
    typedef base::Callback<void(Args...)> then_type;
    
    then_type then_handler_;
    
public:
    this_type& then(then_type&& then_value) {
        then_handler_ = std::move(then_value);
        
        return *this;
    }
    
    void cancel();
};

/*
 *
 */
template <typename... Args>
class Promise : public Future<Args...> {
    enum State {
        PENDING,
        FULFILL,
        REJECT
    };
    
    State state_;
    std::tuple<Args...> value_;
    
    typedef Future<Args...> future_type;
    
public:
    Promise() : Future<Args...>(), state_(PENDING) {
    
    }
    
    void resolve(Args&&... args) {
        future_type::then_handler_.Run(args...);
        future_type::then_handler_.Reset();
    }
    
}; // end of class promise

} // end of namespace


#endif /* ph_apromise_h */
