// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/base_switches.h"

namespace switches {

// If the program includes base/debug/debug_on_start_win.h, the process will
// (on Windows only) start the JIT system-registered debugger on itself and
// will wait for 60 seconds for the debugger to attach to itself. Then a break
// point will be hit.
const char kDebugOnStart[]                  = "debug-on-start";

// Disables the crash reporting.
const char kDisableBreakpad[]               = "disable-breakpad";

// Indicates that crash reporting should be enabled. On platforms where helper
// processes cannot access to files needed to make this decision, this flag is
// generated internally.
const char kEnableCrashReporter[]           = "enable-crash-reporter";

// Enable DCHECKs in release mode.
const char kEnableDCHECK[]                  = "enable-dcheck";

// Generates full memory crash dump.
const char kFullMemoryCrashReport[]         = "full-memory-crash-report";

// Suppresses all error dialogs when present.
const char kNoErrorDialogs[]                = "noerrdialogs";

// When running certain tests that spawn child processes, this switch indicates
// to the test framework that the current process is a child process.
const char kTestChildProcess[]              = "test-child-process";

// Gives the default maximal active V-logging level; 0 is the default.
// Normally positive values are used for V-logging levels.
const char kV[]                             = "v";

// Gives the per-module maximal V-logging levels to override the value
// given by --v.  E.g. "my_module=2,foo*=3" would change the logging
// level for all code in source files "my_module.*" and "foo*.*"
// ("-inl" suffixes are also disregarded for this matching).
//
// Any pattern containing a forward or backward slash will be tested
// against the whole pathname and not just the module.  E.g.,
// "*/foo/bar/*=2" would change the logging level for all code in
// source files under a "foo/bar" directory.
const char kVModule[]                       = "vmodule";

// Will wait for 60 seconds for a debugger to come to attach to the process.
const char kWaitForDebugger[]               = "wait-for-debugger";

// Sends a pretty-printed version of tracing info to the console.
const char kTraceToConsole[]                = "trace-to-console";

// Configure whether chrome://profiler will contain timing information. This
// option is enabled by default. A value of "0" will disable profiler timing,
// while all other values will enable it.
const char kProfilerTiming[]                = "profiler-timing";
// Value of the --profiler-timing flag that will disable timing information for
// chrome://profiler.
const char kProfilerTimingDisabledValue[]   = "0";

#if defined(OS_POSIX)
// Used for turning on Breakpad crash reporting in a debug environment where
// crash reporting is typically compiled but disabled.
const char kEnableCrashReporterForTesting[] =
    "enable-crash-reporter-for-testing";
#endif

#if defined(OS_ANDROID)
// Overrides low-end device detection, disabling low-end device optimizations.
const char kDisableLowEndDeviceMode[]       = "disable-low-end-device-mode";

// Overrides low-end device detection, enabling low-end device optimizations.
const char kEnableLowEndDeviceMode[]        = "enable-low-end-device-mode";
#endif

}  // namespace switches
