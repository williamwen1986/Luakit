#include "common/business_main_delegate.h"

#include "base/command_line.h"
//#include "base/process_util.h"
#include "base/debug/stack_trace.h"
#include "base/message_loop/message_loop.h"
#include "common/logging_chrome.h"
#include "common/business_switches.h"
#include "base/metrics/statistics_recorder.h"

//static SingletonProgressLock s_progress_lock;

bool BusinessMainDelegate::BasicStartupComplete(int *exit_code) {
//  这里可以实现输出版本号/检查升级这种非主功能然后退出的逻辑
//  const CommandLine& command_line = *CommandLine::ForCurrentProcess();
//  
//  if (HandleVersionSwitches(command_line)) {
//    *exit_code = 0;
//    return true;  // Got a --version switch; exit with a success error code.
//  }
//  if (!s_progress_lock.lock()) {
//    *exit_code = 1;
//    return true;
//  }
//
//  *exit_code = 0;
  return false;
}

void BusinessMainDelegate::PreSandboxStartup() {
#if defined(OS_IOS)
  base::debug::EnableInProcessStackDumping();
#endif

  const CommandLine& command_line = *CommandLine::ForCurrentProcess();

  if (command_line.HasSwitch(switches::kMessageLoopHistogrammer)) {
    base::MessageLoop::EnableHistogrammer(true);
//    base::StatisticsRecorder::set_dump_on_exit(true);
  }

  //删除log文件不在这里操作
  logging::OldFileDeletionState file_state = logging::APPEND_TO_OLD_LOG_FILE;

  // 初始化log
  logging::InitChromeLogging(command_line, file_state);

}

void  BusinessMainDelegate::PostCreateThreads(){
//  main::LogFileCleaner::CleanLogFile();
}

void BusinessMainDelegate::ProcessExiting() {
  //logging::CleanupChromeLogging();
}
