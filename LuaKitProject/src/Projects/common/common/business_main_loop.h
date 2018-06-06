// port from chromium

#ifndef CONTENT_BROWSER_BROWSER_MAIN_LOOP_H_
#define CONTENT_BROWSER_BROWSER_MAIN_LOOP_H_
#pragma once

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "common/business_client_process_sub_thread.h"

class CommandLine;
class SystemMessageWindowWin;

namespace base {
class HighResolutionTimerManager;
class MessageLoop;
class SystemMonitor;
}

namespace media {
class AudioManager;
}

namespace net {
class NetworkChangeNotifier;
}

namespace speech {
class SpeechRecognitionManagerImpl;
}


namespace content {
  class MainFunctionParams;
  class BusinessMainParts;
}


class BusinessShutdownImpl;
class BusinessThreadImpl;
struct MainFunctionParams;
class ResourceDispatcherHostImpl;
class WebKitThread;

// Implements the main browser loop stages called from BrowserMainRunner.
// See comments in browser_main_parts.h for additional info.
// All functions are to be called only on the UI thread unless otherwise noted.
class BusinessMainLoop {
 public:
  explicit BusinessMainLoop();

  virtual ~BusinessMainLoop();

  void Init();

  void EarlyInitialization();
  void InitializeToolkit();
  void MainMessageLoopStart();

  // Create all secondary threads.
  void CreateThreads();
    
  BusinessThreadID CreateNewThread(BusinessThread::ID type, const char *threadName);

  // Perform the default message loop run logic.
  void RunMainMessageLoopParts();

  // Performs the shutdown sequence, starting with PostMainMessageLoopRun
  // through stopping threads to PostDestroyThreads.
  void ShutdownThreadsAndCleanUp();

  int GetResultCode() const { return result_code_; }

  // Can be called on any thread.
  static media::AudioManager* GetAudioManager();

 private:
  // For ShutdownThreadsAndCleanUp.
  friend class BusinessShutdownImpl;

  void InitializeMainThread();

  // Called right after the browser threads have been started.
  void BusinessThreadsStarted();

  void MainMessageLoopRun();

  // Members initialized on construction ---------------------------------------
//  const ProtocolCore::MainFunctionParams& parameters_;
//  const CommandLine& parsed_command_line_;
  int result_code_;

  // Members initialized in |MainMessageLoopStart()| ---------------------------
  scoped_ptr<base::MessageLoop> main_message_loop_;
  //scoped_ptr<base::SystemMonitor> system_monitor_;
  scoped_ptr<base::HighResolutionTimerManager> hi_res_timer_manager_;

  // Destroy parts_ before main_message_loop_ (required) and before other
  // classes constructed in content (but after main_thread_).
  scoped_ptr<content::BusinessMainParts> parts_;

  // Members initialized in |InitializeMainThread()| ---------------------------
  // This must get destroyed before other threads that are created in parts_.
  scoped_ptr<BusinessThreadImpl> main_thread_;

  // Members initialized in |RunMainMessageLoopParts()| ------------------------
  scoped_ptr<BusinessProcessSubThread> db_thread_;
  scoped_ptr<BusinessProcessSubThread> file_user_blocking_thread_;
  scoped_ptr<BusinessProcessSubThread> file_thread_;
  scoped_ptr<BusinessProcessSubThread> process_launcher_thread_;
  scoped_ptr<BusinessProcessSubThread> cache_thread_;
  scoped_ptr<BusinessProcessSubThread> io_thread_;
  scoped_ptr<BusinessProcessSubThread> logic_thread_;
    
  std::vector<BusinessProcessSubThread *> moreThreads;
  
#if defined(OS_WIN)
  ui::ScopedOleInitializer ole_initializer_;
#endif

  DISALLOW_COPY_AND_ASSIGN(BusinessMainLoop);
};


#endif  // CONTENT_BROWSER_BROWSER_MAIN_LOOP_H_
