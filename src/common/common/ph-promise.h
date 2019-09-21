//
//  ph_apromise.h
//  common
//
//  Created by zhd32 on 2016/11/8.
//
//

#ifndef apromise_h
#define apromise_h

namespace ph {
  
/*
 *
 */
template <class R>
class promise {
    typedef promise<R> this_type;

public:
    this_type& then();
    
    this_type& fail();
    
}; // end of class promise

} // end of namespace


#endif /* apromise_h */
