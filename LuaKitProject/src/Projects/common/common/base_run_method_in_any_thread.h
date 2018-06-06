#ifndef __BASE_RUN_METHOD_IN_ANY_THREAD_H__
#define __BASE_RUN_METHOD_IN_ANY_THREAD_H__

#include <functional>
#include "base/bind.h"
#include "common/base_lambda_support.h"
#include "common/business_client_thread_impl.h"


#define RunTaskOfVoidReturn() if (MailThread::CurrentlyOn(MailThread::DB)) \
RunMethodInDBThread.Run() ; \
else \
BusinessThread::PostTask(MailThread::DB, FROM_HERE, RunMethodInDBThread) ;


namespace base {
    
    /*template<typename Method>
    inline void RunMethodInDBThreadPostTaskBlock(const Method& method)
    {
        if (BusinessThread::CurrentlyOn(BusinessThread::DB))
            method.Run() ;
        else
        {
            BOOL stop = NO ;
            NSRunLoop* run_loop = [NSRunLoop currentRunLoop];
            auto RunMethodInDBThreadWithQuit = base::BindLambda([=, &stop]()
                                                                {
                                                                    method.Run() ;
                                                                    CFRunLoopStop([run_loop getCFRunLoop]);
                                                                    stop = YES ;
                                                                }) ;
            BusinessThread::PostTask(BusinessThread::DB, FROM_HERE, RunMethodInDBThreadWithQuit);
            while (NO == stop)
                [run_loop runMode:NSRunLoopCommonModes beforeDate:[NSDate distantFuture]];
        }
    }*/
    
    template<typename Method>
    inline void RunMethodInDBThreadPostTaskBlock(const Method& method) {
        if (BusinessThread::CurrentlyOn(BusinessThread::DB))
            method.Run() ;
        else
        {
            dispatch_semaphore_t s = dispatch_semaphore_create(0) ;
            
            auto RunMethodInDBThreadWithQuit = base::BindLambda([=]()
                                                                {
                                                                    method.Run() ;
                                                                    dispatch_semaphore_signal(s) ;
                                                                }) ;
            BusinessThread::PostTask(BusinessThread::DB, FROM_HERE, RunMethodInDBThreadWithQuit);
            
            dispatch_semaphore_wait(s, DISPATCH_TIME_FOREVER) ;
            //dispatch_release(s) ;
        }
    }
}

#endif
