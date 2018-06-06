#ifndef ph_promise_internal_h
#define ph_promise_internal_h

#include <tuple>
#include <type_traits>

namespace ph {

namespace internal {

template <typename Functor, typename... BoundArgs>
struct BindState {

    //
    // constructor
    //
    explicit BindState() {
        // todo: check
    }
    
    Functor functor_; // store for functor
    std::tuple<BoundArgs...> bound_args_; // store for args
};


} // end of namespace internal
  
  
} // end of namespace ph



#endif /* ph_promise_internal_h */
