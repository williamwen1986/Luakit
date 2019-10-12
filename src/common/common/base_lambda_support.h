#ifndef __BASE_LAMBDA_SUPPORT_H__
#define __BASE_LAMBDA_SUPPORT_H__

#include <functional>
#include "base/bind.h"
#include "base/memory/weak_ptr.h"
#if defined(__OBJC__)
#import <Foundation/Foundation.h>
#endif

namespace base {
    /**
     * @brief @c BindLambda 函数实现了便捷的通过 C++ @c Lambda 表达式来创建 @c base::Callback 的方法。
     *
     * @param functor C++ @c Lambda 表达式
     * @param params 需要绑定在 @c Lambda 表达式上的参数
     *
     * @note 可根据实际情况，选择使用捕获或者绑定的方式传递参数。
     */
    template <typename Functor, typename ...Params>
    auto BindLambda(const Functor& functor, const Params&... params) -> decltype(base::Bind(&Functor::operator(), base::Owned(&functor), params...)) {
        auto functor_on_heap = new Functor(functor);
        return base::Bind(&Functor::operator(), base::Owned(functor_on_heap), params...);
    }
    
    template <typename SupportWeakPtrType, typename ...Params>
    void _RunCallbackInternal(const WeakPtr<SupportWeakPtrType>& weakptr, const base::Callback<void(Params...)>& callback, Params... params) {
        if (weakptr.get()) {
            callback.Run(params...);
        }
    }
    
    template <typename SupportWeakPtrType, typename ...Params>
    base::Callback<void(Params...)> _WrapCallback(const base::Callback<void(Params...)>& callback, const WeakPtr<SupportWeakPtrType>& weakptr) {
        return base::Bind(&_RunCallbackInternal<SupportWeakPtrType, Params...>, weakptr, callback);
    }
    
    /**
     * @brief @c BindLambda 函数实现了便捷的通过 C++ @c Lambda 表达式来创建 @c base::Callback 的方法。\n
     * 这个重载允许额外传入一个 @c base::WeakPtr 类型的弱引用，在实际执行 @p functor 前会检查弱引用的有效性，如果弱引用已经无效，则不会执行 @p functor。\n
     *
     * @param weakptr 额外传递一个弱引用，在 @p functor 执行前会进行检查，如果该弱引用无效则不会继续调用 @p functor
     * @param functor C++ @c Lambda 表达式
     * @param params 需要绑定在 @c Lambda 表达式上的参数
     *
     * @note 可根据实际情况，选择使用捕获或者绑定的方式传递参数。
     */
    template <typename SupportWeakPtrType, typename Functor, typename ...Params>
    auto BindLambda(const WeakPtr<SupportWeakPtrType>& weakptr, const Functor& functor, const Params&... params) -> decltype(BindLambda(functor, params...)) {
        return _WrapCallback(BindLambda(functor, params...), weakptr);
    }
    
#if defined(__OBJC__)
    template <typename ...Params>
    void _RunCallbackInternal_ObjC(NSHashTable *weakptr_container, const base::Callback<void(Params...)>& callback, Params... params) {
        if ([weakptr_container anyObject]) {
            callback.Run(params...);
        }
    }

    template <typename ...Params>
    base::Callback<void(Params...)> _WrapCallback_ObjC(const base::Callback<void(Params...)>& callback, NSHashTable *weakptr_container) {
        return base::Bind(&_RunCallbackInternal_ObjC<Params...>, weakptr_container, callback);
    }
    
    /**
     * @brief @c BindLambda 函数实现了便捷的通过 C++ @c Lambda 表达式来创建 @c base::Callback 的方法。\n
     * 这个重载允许额外传入一个 @c id 类型的对象，在实际执行 @p functor 前会检查该对象的有效性，如果该对象已经无效，则不会执行 @p functor。
     *
     * @param object 额外传递一个对象，在 @p functor 执行前会进行检查，如果该对象无效则不会继续调用 @p functor
     * @param functor C++ @c Lambda 表达式
     * @param params 需要绑定在 @c Lambda 表达式上的参数
     *
     * @note 可根据实际情况，选择使用捕获或者绑定的方式传递参数。
     */
    template <typename Functor, typename ...Params>
    auto BindLambda(id object, const Functor& functor, const Params&... params) -> decltype(BindLambda(functor, params...)) {
        NSHashTable *hashTable = [NSHashTable weakObjectsHashTable];
        [hashTable addObject:object];
        return _WrapCallback_ObjC(BindLambda(functor, params...), hashTable);
    }
#endif
}

#endif
