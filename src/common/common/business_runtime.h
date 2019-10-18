#ifndef _MAIL_MAIN_RUNNER_H_
#define _MAIL_MAIN_RUNNER_H_
#pragma once

#include "base/at_exit.h"
#include "common/business_client_thread.h"
namespace ProtocolCore {
  struct MainFunctionParams;
}

class BusinessMainDelegate;

// This class is responsible for foxmail initialization, running and shutdown.
class BusinessRuntime {
 public:
  virtual ~BusinessRuntime() {}

  // Create a new FoxmailMainRunner object.
  static BusinessRuntime* Create();
    
  static BusinessRuntime* GetRuntime();

  // Initialize all necessary browser state. The |parameters| values will be
  // copied.
  virtual int Initialize(BusinessMainDelegate *delegate) = 0;

  // Perform the default run logic.
  virtual int Run() = 0;
    
  // Perform the default run logic.
  virtual BusinessThreadID createNewThread(BusinessThread::ID type, const char *threadName) = 0;

  // Shut down the browser state.
  virtual void Shutdown() = 0;
};


#endif  // _MAIL_MAIN_RUNNER_H_
