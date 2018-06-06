// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "build/build_config.h"
#include "common/business_main_loop.h"

#include "base/bind.h"
#include "base/debug/trace_event.h"
#include "base/timer/hi_res_timer_manager.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/metrics/field_trial.h"
#include "base/metrics/histogram.h"
#include "base/strings/string_number_conversions.h"
#include "base/threading/thread_restrictions.h"
//#include "crypto/nss_util.h"
#include "common/business_client_thread_impl.h"
#include "common/business_client_main_parts.h"
#include "common/business_client_shutdown.h"
#include "common/result_codes.h"
#include "main_function_params.h"
// Friendly names for the well-known threads.
static std::vector<const char*> g_browser_thread_names = {
    "",  // UI (name assembled in browser_main.cc).
    "Business_DBThread",  // DB
    "Business_LogicThread",  // Logic
    //  "Business_WebKitThread",  // WEBKIT_DEPRECATED
    "Business_FileThread",  // FILE
    //  "Business_FileUserBlockingThread",  // FILE_USER_BLOCKING
    //  "Business_ProcessLauncherThread",  // PROCESS_LAUNCHER
    //  "Business_CacheThread",  // CACHE
    "Business_IOThread",  // IO
};
// The currently-running BusinessMainLoop.  There can be one or zero.
BusinessMainLoop* g_current_browser_main_loop = NULL;

// This is just to be able to keep ShutdownThreadsAndCleanUp out of
// the public interface of BusinessMainLoop.
class BusinessShutdownImpl {
 public:
  static void ImmediateShutdownAndExitProcess() {
    DCHECK(g_current_browser_main_loop);
    g_current_browser_main_loop->ShutdownThreadsAndCleanUp();

    NOTIMPLEMENTED();
  }
};

void ImmediateShutdownAndExitProcess() {
  BusinessShutdownImpl::ImmediateShutdownAndExitProcess();
}

// BusinessMainLoop construction / destruction =============================

BusinessMainLoop::BusinessMainLoop()
    : result_code_(content::RESULT_CODE_NORMAL_EXIT) {
  DCHECK(!g_current_browser_main_loop);
  g_current_browser_main_loop = this;
}

BusinessMainLoop::~BusinessMainLoop() {
  DCHECK_EQ(this, g_current_browser_main_loop);
  g_current_browser_main_loop = NULL;
}

void BusinessMainLoop::Init() {
#if defined(OS_MACOSX)
//  FoxmailMainParts *main_parts = new FoxmailMainPartsMac(parameters_);
 // parts_.reset(main_parts);
#endif
}

// BusinessMainLoop stages ==================================================

void BusinessMainLoop::EarlyInitialization() {
#if defined(USE_X11)
  if (parsed_command_line_.HasSwitch(switches::kSingleProcess) ||
      parsed_command_line_.HasSwitch(switches::kInProcessGPU)) {
    if (!XInitThreads()) {
      LOG(ERROR) << "Failed to put Xlib into threaded mode.";
    }
  }
#endif

  if (parts_.get())
    parts_->PreEarlyInitialization();

#if defined(OS_WIN)
  net::EnsureWinsockInit();
#endif

#if !defined(USE_OPENSSL)
  // Use NSS for SSL by default.
  // The default client socket factory uses NSS for SSL by default on
  // Windows and Mac.
  bool init_nspr = false;
#if defined(OS_WIN) || defined(OS_MACOSX)
//  if (parsed_command_line_.HasSwitch(switches::kUseSystemSSL)) {
//    net::ClientSocketFactory::UseSystemSSL();
//  } else {
//    init_nspr = true;
//  }
  UMA_HISTOGRAM_BOOLEAN("Foxmail.CommandLineUseSystemSSL", !init_nspr);
#elif defined(USE_NSS)
  init_nspr = true;
#endif
  if (init_nspr) {
    // We want to be sure to init NSPR on the main thread.
//    crypto::EnsureNSPRInit();
  }
#endif  // !defined(USE_OPENSSL)
  
#if defined(OS_POSIX) && !defined(OS_MACOSX) && !defined(OS_ANDROID)
  SetupSandbox(parsed_command_line_);
#endif

  
#if 0
  if (parsed_command_line_.HasSwitch(switches::kEnableSSLCachedInfo))
    net::SSLConfigService::EnableCachedInfo();

  // TODO(abarth): Should this move to InitializeNetworkOptions?  This doesn't
  // seem dependent on SSL initialization().
  if (parsed_command_line_.HasSwitch(switches::kEnableTcpFastOpen))
    net::set_tcp_fastopen_enabled(true);

  if (parsed_command_line_.HasSwitch(switches::kRendererProcessLimit)) {
    std::string limit_string = parsed_command_line_.GetSwitchValueASCII(
        switches::kRendererProcessLimit);
    size_t process_limit;
    if (base::StringToSizeT(limit_string, &process_limit)) {
      content::RenderProcessHost::SetMaxRendererProcessCount(process_limit);
    }
  }
#endif
  
  if (parts_.get())
    parts_->PostEarlyInitialization();
}

void BusinessMainLoop::MainMessageLoopStart() {
  if (parts_.get())
    parts_->PreMainMessageLoopStart();

  // Create a MessageLoop if one does not already exist for the current thread.
  if (!base::MessageLoop::current())
    main_message_loop_.reset(new base::MessageLoop(base::MessageLoop::TYPE_UI));

  InitializeMainThread();

  hi_res_timer_manager_.reset(new base::HighResolutionTimerManager);

  if (parts_.get())
    parts_->PostMainMessageLoopStart();
}

BusinessThreadID BusinessMainLoop::CreateNewThread(BusinessThread::ID type, const char *threadName) {
    g_browser_thread_names.push_back(threadName);
    base::Thread::Options default_options;
    base::Thread::Options io_message_loop_options;
    io_message_loop_options.message_loop_type = base::MessageLoop::TYPE_IO;
    base::Thread::Options ui_message_loop_options;
    ui_message_loop_options.message_loop_type = base::MessageLoop::TYPE_UI;
    base::Thread::Options* options = &default_options;
    switch (type) {
        case BusinessThread::DB:
            break;
        case BusinessThread::LOGIC:
            break;
        case BusinessThread::FILE:
            options = &io_message_loop_options;
            break;
        case BusinessThread::IO:
            options = &io_message_loop_options;
            break;
        case BusinessThread::UI:
        case BusinessThread::ID_COUNT:
        default:
            NOTREACHED();
            break;
    }
    BusinessProcessSubThread * threadPtr;
    BusinessThreadID newId = BusinessThread::getThreadCount();
    threadPtr = new BusinessProcessSubThread((BusinessThreadID)newId, threadName);
    threadPtr->StartWithOptions(*options);
    moreThreads.push_back(threadPtr);
    return newId;
}

void BusinessMainLoop::CreateThreads() {
  if (parts_.get())
    result_code_ = parts_->PreCreateThreads();

  if (result_code_ > 0)
    return;

  base::Thread::Options default_options;
  base::Thread::Options io_message_loop_options;
  io_message_loop_options.message_loop_type = base::MessageLoop::TYPE_IO;
  base::Thread::Options ui_message_loop_options;
  ui_message_loop_options.message_loop_type = base::MessageLoop::TYPE_UI;

  // Start threads in the order they occur in the BusinessThreadID
  // enumeration, except for BusinessThread::UI which is the main
  // thread.
  //
  // Must be size_t so we can increment it.
  for (size_t thread_id = BusinessThread::UI + 1;
       thread_id < BusinessThread::ID_COUNT;
       ++thread_id) {
    scoped_ptr<BusinessProcessSubThread>* thread_to_start = NULL;
    base::Thread::Options* options = &default_options;

    switch (thread_id) {
      case BusinessThread::DB:
        thread_to_start = &db_thread_;
        break;
      case BusinessThread::LOGIC:
        thread_to_start = &logic_thread_;
        break;
//      case BusinessThread::FILE_USER_BLOCKING:
//        thread_to_start = &file_user_blocking_thread_;
//        break;
      case BusinessThread::FILE:
        thread_to_start = &file_thread_;
        options = &io_message_loop_options;
        break;
//      case BusinessThread::PROCESS_LAUNCHER:
//        thread_to_start = &process_launcher_thread_;
//        break;
//      case BusinessThread::CACHE:
//        thread_to_start = &cache_thread_;
//        options = &io_message_loop_options;
//        break;
      case BusinessThread::IO:
        thread_to_start = &io_thread_;
        options = &io_message_loop_options;
        break;
      case BusinessThread::UI:
      case BusinessThread::ID_COUNT:
      default:
        NOTREACHED();
        break;
    }

    BusinessThreadID id = static_cast<BusinessThreadID>(thread_id);

    if (thread_to_start) {
      (*thread_to_start).reset(new BusinessProcessSubThread(id, g_browser_thread_names[id]));
      (*thread_to_start)->StartWithOptions(*options);
    } else {
      NOTREACHED();
    }
  }

//  BrowserGpuChannelHostFactory::Initialize();
#if defined(USE_AURA)
  ImageTransportFactory::Initialize();
#endif

  BusinessThreadsStarted();

  if (parts_.get())
    parts_->PreMainMessageLoopRun();

  // If the UI thread blocks, the whole UI is unresponsive.
  // Do not allow disk IO from the UI thread.
  base::ThreadRestrictions::SetIOAllowed(false);
  base::ThreadRestrictions::DisallowWaiting();
}

void BusinessMainLoop::RunMainMessageLoopParts() {
  TRACE_EVENT_BEGIN_ETW("BusinessMain:MESSAGE_LOOP", 0, "");

  bool ran_main_loop = false;
  if (parts_.get())
    ran_main_loop = parts_->MainMessageLoopRun(&result_code_);

  if (!ran_main_loop)
    MainMessageLoopRun();

  TRACE_EVENT_END_ETW("BusinessMain:MESSAGE_LOOP", 0, "");
}

void BusinessMainLoop::ShutdownThreadsAndCleanUp() {
  // Teardown may start in PostMainMessageLoopRun, and during teardown we
  // need to be able to perform IO.
  base::ThreadRestrictions::SetIOAllowed(true);
//  BusinessThread::PostTask(
//      BusinessThread::IO, FROM_HERE,
//      base::Bind(base::IgnoreResult(&base::ThreadRestrictions::SetIOAllowed),
//                 true));

  if (parts_.get())
    parts_->PostMainMessageLoopRun();

  // Must be size_t so we can subtract from it.
  for (int i = (int)moreThreads.size()-1; i>=0; i--) {
      BusinessProcessSubThread* thread_to_stop = moreThreads[i];
      if (thread_to_stop) {
          delete thread_to_stop;
          moreThreads[i] = NULL;
      } else {
          NOTREACHED();
      }
  }
    
  for (size_t thread_id = BusinessThread::ID_COUNT - 1;
       thread_id >= (BusinessThread::UI + 1);
       --thread_id) {
    // Find the thread object we want to stop. Looping over all valid
    // BusinessThread IDs and DCHECKing on a missing case in the switch
    // statement helps avoid a mismatch between this code and the
    // BusinessThread::ID enumeration.
    //
    // The destruction order is the reverse order of occurrence in the
    // BusinessThread::ID list. The rationale for the order is as
    // follows (need to be filled in a bit):
    //
    //
    // - The IO thread is the only user of the CACHE thread.
    //
    // - The PROCESS_LAUNCHER thread must be stopped after IO in case
    //   the IO thread posted a task to terminate a process on the
    //   process launcher thread.
    //
    // - (Not sure why FILE needs to stop before WEBKIT.)
    //
    // - The WEBKIT thread (which currently is the responsibility of
    //   the embedder to stop, by destroying ResourceDispatcherHost
    //   before the DB thread is stopped)
    //
    // - (Not sure why DB stops last.)
    scoped_ptr<BusinessProcessSubThread>* thread_to_stop = NULL;
    switch (thread_id) {
      case BusinessThread::DB:
        thread_to_stop = &db_thread_;
        break;
      case BusinessThread::LOGIC:
        thread_to_stop = &logic_thread_;
        break;
//      case BusinessThread::FILE_USER_BLOCKING:
//        thread_to_stop = &file_user_blocking_thread_;
//        break;
      case BusinessThread::FILE:
        thread_to_stop = &file_thread_;
        break;
//      case BusinessThread::PROCESS_LAUNCHER:
//        thread_to_stop = &process_launcher_thread_;
//        break;
//      case BusinessThread::CACHE:
//        thread_to_stop = &cache_thread_;
//        break;
      case BusinessThread::IO:
        thread_to_stop = &io_thread_;
        break;
      case BusinessThread::UI:
      case BusinessThread::ID_COUNT:
      default:
        NOTREACHED();
        break;
    }

//    BusinessThread::ID id = static_cast<BusinessThread::ID>(thread_id);

    /*if (id == BusinessThread::WEBKIT_DEPRECATED) {
      webkit_thread_.reset();
    } else*/ if (thread_to_stop) {
      thread_to_stop->reset();
    } else {
      NOTREACHED();
    }
  }

  // Close the blocking I/O pool after the other threads. Other threads such
  // as the I/O thread may need to schedule work like closing files or flushing
  // data during shutdown, so the blocking pool needs to be available. There
  // may also be slow operations pending that will blcok shutdown, so closing
  // it here (which will block until required operations are complete) gives
  // more head start for those operations to finish.
  BusinessThreadImpl::ShutdownThreadPool();

  if (parts_.get())
    parts_->PostDestroyThreads();
    
#if defined(OS_IOS)
    base::MessageLoopForUI::current()->Detach();
#else
    //assert(false);
#endif
}

void BusinessMainLoop::InitializeMainThread() {
  const char* kThreadName = "Business_MainThread";
  base::PlatformThread::SetName(kThreadName);
  if (main_message_loop_.get())
    main_message_loop_->set_thread_name(kThreadName);

  // Register the main thread by instantiating it, but don't call any methods.
  main_thread_.reset(new BusinessThreadImpl(BusinessThread::UI,
                                           base::MessageLoop::current()));

#ifdef OS_ANDROID
    //betahuang : android程序中要显示调用MessageLoopForUI的Start函数，初始化相关变量
    if (base::MessageLoopForUI::current()) {
        base::MessageLoopForUI::current()->Start();
    }
#endif
}


void BusinessMainLoop::BusinessThreadsStarted() {
}

void BusinessMainLoop::InitializeToolkit() {
  // TODO(evan): this function is rather subtle, due to the variety
  // of intersecting ifdefs we have.  To keep it easy to follow, there
  // are no #else branches on any #ifs.
  // TODO(stevenjb): Move platform specific code into platform specific Parts
  // (Need to add InitializeToolkit stage to BrowserParts).
  
  if (parts_.get())
    parts_->ToolkitInitialized();
}

void BusinessMainLoop::MainMessageLoopRun() {
//  if (parameters_.ui_task)
//    base::MessageLoopForUI::current()->PostTask(FROM_HERE, *parameters_.ui_task);

#if defined(OS_IOS)
    base::MessageLoopForUI::current()->Attach();
#elif !defined(OS_ANDROID) //android不需要调用Run
  base::MessageLoopForUI::current()->Run();
#endif
}

