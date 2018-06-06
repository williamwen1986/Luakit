#include <signal.h>

#include "common/business_runtime.h"
#include "base/allocator/allocator_shim.h"
#include "base/at_exit.h"
#include "base/base_switches.h"
#include "base/strings/stringprintf.h"
#include "base/debug/trace_event.h"
#include "base/logging.h"
#include "base/mac/scoped_nsautorelease_pool.h"
#include "base/memory/scoped_ptr.h"
#include "base/metrics/histogram.h"
#include "base/metrics/statistics_recorder.h"
#include "base/metrics/stats_table.h"
#include "common/business_switches.h"
#include "common/notification_service.h"
#include "common/business_main_loop.h"
#include "common/business_main_delegate.h"
#include "common/main_function_params.h"
#include "base/path_service.h"
#include "base/files/file_path.h"

bool g_exited_main_message_loop = false;

static BusinessRuntime* MainRunTime = NULL;

//static void InitializeStatsTable(const CommandLine& command_line) {
//  const char    kStatsFilename[] = "FoxmailStats";
//  const int kStatsMaxThreads = 32;
//  const int kStatsMaxCounters = 3000;
//
//  // Initialize the Stats Counters table.  With this initialized,
//  // the StatsViewer can be utilized to read counters outside of
//  // Chrome.  These lines can be commented out to effectively turn
//  // counters 'off'.  The table is created and exists for the life
//  // of the process.  It is not cleaned up.
//  if (command_line.HasSwitch(switches::kEnableStatsTable)) {
//    // NOTIMPLEMENTED: we probably need to shut this down correctly to avoid
//    // leaking shared memory regions on posix platforms.
//    std::string statsfile =
//      base::StringPrintf("%s", kStatsFilename);
//    base::StatsTable* stats_table = new base::StatsTable(statsfile,
//        kStatsMaxThreads, kStatsMaxCounters);
//    base::StatsTable::set_current(stats_table);
//  }
//}

class BusinessRuntimeImpl : public BusinessRuntime {
 public:
  BusinessRuntimeImpl()
      : is_initialized_(false),
        is_shutdown_(false),
        created_threads_(false) {
  }

  ~BusinessRuntimeImpl() {
    if (is_initialized_ && !is_shutdown_)
      Shutdown();
  }
    
  virtual BusinessThreadID createNewThread(BusinessThread::ID type, const char *threadName)
    OVERRIDE {
        return main_loop_->CreateNewThread(type, threadName);
  }
  
  virtual int Initialize(BusinessMainDelegate *delegate)
      OVERRIDE {
    is_initialized_ = true;
    delegate_ = delegate;
    
    // The exit manager is in charge of calling the dtors of singleton objects.
    exit_manager_.reset(new base::AtExitManager);

    base::StatisticsRecorder::Initialize();
          
    notification_service_.reset(content::NotificationService::Create());
    
    int exit_code = 0;
    if (delegate && delegate->BasicStartupComplete(&exit_code))
      return exit_code;
  
    if (delegate)
       delegate->PreSandboxStartup();
    
    // We need this pool for all the objects created before we get to the
    // event loop, but we don't want to leave them hanging around until the
    // app quits. Each "main" needs to flush this pool right before it goes into
    // its main event loop to get rid of the cruft.
#ifndef OS_ANDROID          
//    autorelease_pool_.reset(new base::mac::ScopedNSAutoreleasePool());
#endif
    //InitializeStatsTable(parameters.command_line);
    // InitializeSandbox()
       
    if (delegate)
      delegate->SandboxInitialized();
       
    main_loop_.reset(new BusinessMainLoop());

    main_loop_->Init();

    main_loop_->EarlyInitialization();

    // Must happen before we try to use a message loop or display any UI.
    main_loop_->InitializeToolkit();

    main_loop_->MainMessageLoopStart();

    // WARNING: If we get a WM_ENDSESSION, objects created on the stack here
    // are NOT deleted. If you need something to run during WM_ENDSESSION add it
    // to browser_shutdown::Shutdown or FoxmailProcess::EndSession.

    main_loop_->CreateThreads();
    if (delegate) {
      delegate->PostCreateThreads();
    }

    int result_code = main_loop_->GetResultCode();
    if (result_code > 0)
      return result_code;
    created_threads_ = true;

    // sigpipe
    signal(SIGPIPE, SIG_IGN);

    // Return -1 to indicate no early termination.
    return -1;
  }

  virtual int Run() OVERRIDE {
    DCHECK(is_initialized_);
    DCHECK(!is_shutdown_);
    main_loop_->RunMainMessageLoopParts();
    return main_loop_->GetResultCode();
  }

  virtual void Shutdown() OVERRIDE {
    DCHECK(is_initialized_);
    DCHECK(!is_shutdown_);
    g_exited_main_message_loop = true;

    if (created_threads_)
      main_loop_->ShutdownThreadsAndCleanUp();
    
    if (delegate_)
      delegate_->ProcessExiting();

    main_loop_.reset(NULL);
#ifndef OS_ANDROID
//    autorelease_pool_.reset(NULL);
#endif
    notification_service_.reset(NULL);
    exit_manager_.reset(NULL);

    delegate_ = NULL;
    is_shutdown_ = true;
  }

 protected:
  // True if the runner has been initialized.
  bool is_initialized_;

  // True if the runner has been shut down.
  bool is_shutdown_;

  // True if the non-UI threads were created.
  bool created_threads_;

  scoped_ptr<base::AtExitManager> exit_manager_;
  scoped_ptr<BusinessMainLoop> main_loop_;
#ifndef OS_ANDROID
//  scoped_ptr<base::mac::ScopedNSAutoreleasePool> autorelease_pool_;
#endif
  scoped_ptr<content::NotificationService> notification_service_;
  
  BusinessMainDelegate *delegate_;

  DISALLOW_COPY_AND_ASSIGN(BusinessRuntimeImpl);
};

// static
BusinessRuntime* BusinessRuntime::Create() {
  MainRunTime = new BusinessRuntimeImpl();
  return MainRunTime;
}

BusinessRuntime* BusinessRuntime::GetRuntime()
{
    return MainRunTime;
}

