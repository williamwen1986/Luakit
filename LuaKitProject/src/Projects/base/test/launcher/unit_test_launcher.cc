// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/test/launcher/unit_test_launcher.h"

#include "base/bind.h"
#include "base/callback_helpers.h"
#include "base/command_line.h"
#include "base/compiler_specific.h"
#include "base/debug/debugger.h"
#include "base/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/format_macros.h"
#include "base/message_loop/message_loop.h"
#include "base/stl_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/sys_info.h"
#include "base/test/gtest_xml_util.h"
#include "base/test/launcher/test_launcher.h"
#include "base/test/test_switches.h"
#include "base/test/test_timeouts.h"
#include "base/third_party/dynamic_annotations/dynamic_annotations.h"
#include "base/threading/thread_checker.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace base {

namespace {

// This constant controls how many tests are run in a single batch by default.
const size_t kDefaultTestBatchLimit = 10;

const char kHelpFlag[] = "help";

// Flag to run all tests in a single process.
const char kSingleProcessTestsFlag[] = "single-process-tests";

void PrintUsage() {
  fprintf(stdout,
          "Runs tests using the gtest framework, each batch of tests being\n"
          "run in their own process. Supported command-line flags:\n"
          "\n"
          " Common flags:\n"
          "  --gtest_filter=...\n"
          "    Runs a subset of tests (see --gtest_help for more info).\n"
          "\n"
          "  --help\n"
          "    Shows this message.\n"
          "\n"
          "  --gtest_help\n"
          "    Shows the gtest help message.\n"
          "\n"
          "  --test-launcher-jobs=N\n"
          "    Sets the number of parallel test jobs to N.\n"
          "\n"
          "  --single-process-tests\n"
          "    Runs the tests and the launcher in the same process. Useful\n"
          "    for debugging a specific test in a debugger.\n"
          "\n"
          " Other flags:\n"
          "  --test-launcher-batch-limit=N\n"
          "    Sets the limit of test batch to run in a single process to N.\n"
          "\n"
          "  --test-launcher-debug-launcher\n"
          "    Disables autodetection of debuggers and similar tools,\n"
          "    making it possible to use them to debug launcher itself.\n"
          "\n"
          "  --test-launcher-retry-limit=N\n"
          "    Sets the limit of test retries on failures to N.\n"
          "\n"
          "  --test-launcher-summary-output=PATH\n"
          "    Saves a JSON machine-readable summary of the run.\n"
          "\n"
          "  --test-launcher-print-test-stdio=auto|always|never\n"
          "    Controls when full test output is printed.\n"
          "    auto means to print it when the test failed.\n"
          "\n"
          "  --test-launcher-total-shards=N\n"
          "    Sets the total number of shards to N.\n"
          "\n"
          "  --test-launcher-shard-index=N\n"
          "    Sets the shard index to run to N (from 0 to TOTAL - 1).\n");
  fflush(stdout);
}

// Returns command line for child GTest process based on the command line
// of current process. |test_names| is a vector of test full names
// (e.g. "A.B"), |output_file| is path to the GTest XML output file.
CommandLine GetCommandLineForChildGTestProcess(
    const std::vector<std::string>& test_names,
    const base::FilePath& output_file) {
  CommandLine new_cmd_line(*CommandLine::ForCurrentProcess());

  new_cmd_line.AppendSwitchPath(switches::kTestLauncherOutput, output_file);
  new_cmd_line.AppendSwitchASCII(kGTestFilterFlag, JoinString(test_names, ":"));
  new_cmd_line.AppendSwitch(kSingleProcessTestsFlag);

  return new_cmd_line;
}

class UnitTestLauncherDelegate : public TestLauncherDelegate {
 public:
  explicit UnitTestLauncherDelegate(size_t batch_limit)
      : batch_limit_(batch_limit) {
  }

  virtual ~UnitTestLauncherDelegate() {
    DCHECK(thread_checker_.CalledOnValidThread());
  }

 private:
  struct GTestCallbackState {
    TestLauncher* test_launcher;
    std::vector<std::string> test_names;
    FilePath output_file;
  };

  virtual void OnTestIterationStarting() OVERRIDE {
    // Nothing to do.
  }

  virtual std::string GetTestNameForFiltering(
      const testing::TestCase* test_case,
      const testing::TestInfo* test_info) OVERRIDE {
    DCHECK(thread_checker_.CalledOnValidThread());

    return std::string(test_case->name()) + "." + test_info->name();
  }

  virtual bool ShouldRunTest(const testing::TestCase* test_case,
                             const testing::TestInfo* test_info) OVERRIDE {
    DCHECK(thread_checker_.CalledOnValidThread());

    // There is no additional logic to disable specific tests.
    return true;
  }

  virtual size_t RunTests(TestLauncher* test_launcher,
                          const std::vector<std::string>& test_names) OVERRIDE {
    DCHECK(thread_checker_.CalledOnValidThread());

    std::vector<std::string> batch;
    for (size_t i = 0; i < test_names.size(); i++) {
      batch.push_back(test_names[i]);

      if (batch.size() >= batch_limit_) {
        RunBatch(test_launcher, batch);
        batch.clear();
      }
    }

    RunBatch(test_launcher, batch);

    return test_names.size();
  }

  virtual size_t RetryTests(
      TestLauncher* test_launcher,
      const std::vector<std::string>& test_names) OVERRIDE {
    MessageLoop::current()->PostTask(
        FROM_HERE,
        Bind(&UnitTestLauncherDelegate::RunSerially,
             Unretained(this),
             test_launcher,
             test_names));
    return test_names.size();
  }

  void RunSerially(TestLauncher* test_launcher,
                   const std::vector<std::string>& test_names) {
    if (test_names.empty())
      return;

    std::vector<std::string> new_test_names(test_names);
    std::string test_name(new_test_names.back());
    new_test_names.pop_back();

    // Create a dedicated temporary directory to store the xml result data
    // per run to ensure clean state and make it possible to launch multiple
    // processes in parallel.
    base::FilePath output_file;
    CHECK(CreateNewTempDirectory(FilePath::StringType(), &output_file));
    output_file = output_file.AppendASCII("test_results.xml");

    std::vector<std::string> current_test_names;
    current_test_names.push_back(test_name);
    CommandLine cmd_line(
        GetCommandLineForChildGTestProcess(current_test_names, output_file));

    GTestCallbackState callback_state;
    callback_state.test_launcher = test_launcher;
    callback_state.test_names = current_test_names;
    callback_state.output_file = output_file;

    test_launcher->LaunchChildGTestProcess(
        cmd_line,
        std::string(),
        TestTimeouts::test_launcher_timeout(),
        Bind(&UnitTestLauncherDelegate::SerialGTestCallback,
             Unretained(this),
             callback_state,
             new_test_names));
  }

  void RunBatch(TestLauncher* test_launcher,
                const std::vector<std::string>& test_names) {
    DCHECK(thread_checker_.CalledOnValidThread());

    if (test_names.empty())
      return;

    // Create a dedicated temporary directory to store the xml result data
    // per run to ensure clean state and make it possible to launch multiple
    // processes in parallel.
    base::FilePath output_file;
    CHECK(CreateNewTempDirectory(FilePath::StringType(), &output_file));
    output_file = output_file.AppendASCII("test_results.xml");

    CommandLine cmd_line(
        GetCommandLineForChildGTestProcess(test_names, output_file));

    // Adjust the timeout depending on how many tests we're running
    // (note that e.g. the last batch of tests will be smaller).
    // TODO(phajdan.jr): Consider an adaptive timeout, which can change
    // depending on how many tests ran and how many remain.
    // Note: do NOT parse child's stdout to do that, it's known to be
    // unreliable (e.g. buffering issues can mix up the output).
    base::TimeDelta timeout =
        test_names.size() * TestTimeouts::test_launcher_timeout();

    GTestCallbackState callback_state;
    callback_state.test_launcher = test_launcher;
    callback_state.test_names = test_names;
    callback_state.output_file = output_file;

    test_launcher->LaunchChildGTestProcess(
        cmd_line,
        std::string(),
        timeout,
        Bind(&UnitTestLauncherDelegate::GTestCallback,
             Unretained(this),
             callback_state));
  }

  void GTestCallback(const GTestCallbackState& callback_state,
                     int exit_code,
                     const TimeDelta& elapsed_time,
                     bool was_timeout,
                     const std::string& output) {
    DCHECK(thread_checker_.CalledOnValidThread());
    std::vector<std::string> tests_to_relaunch;
    ProcessTestResults(callback_state.test_launcher,
                       callback_state.test_names,
                       callback_state.output_file,
                       output,
                       exit_code,
                       was_timeout,
                       &tests_to_relaunch);

    // Relaunch requested tests in parallel, but only use single
    // test per batch for more precise results (crashes, test passes
    // but non-zero exit codes etc).
    for (size_t i = 0; i < tests_to_relaunch.size(); i++) {
      std::vector<std::string> batch;
      batch.push_back(tests_to_relaunch[i]);
      RunBatch(callback_state.test_launcher, batch);
    }

    // The temporary file's directory is also temporary.
    DeleteFile(callback_state.output_file.DirName(), true);
  }

  void SerialGTestCallback(const GTestCallbackState& callback_state,
                           const std::vector<std::string>& test_names,
                           int exit_code,
                           const TimeDelta& elapsed_time,
                           bool was_timeout,
                           const std::string& output) {
    DCHECK(thread_checker_.CalledOnValidThread());
    std::vector<std::string> tests_to_relaunch;
    bool called_any_callbacks =
        ProcessTestResults(callback_state.test_launcher,
                           callback_state.test_names,
                           callback_state.output_file,
                           output,
                           exit_code,
                           was_timeout,
                           &tests_to_relaunch);

    // There is only one test, there cannot be other tests to relaunch
    // due to a crash.
    DCHECK(tests_to_relaunch.empty());

    // There is only one test, we should have called back with its result.
    DCHECK(called_any_callbacks);

    // The temporary file's directory is also temporary.
    DeleteFile(callback_state.output_file.DirName(), true);

    MessageLoop::current()->PostTask(
        FROM_HERE,
        Bind(&UnitTestLauncherDelegate::RunSerially,
             Unretained(this),
             callback_state.test_launcher,
             test_names));
  }

  static bool ProcessTestResults(
      TestLauncher* test_launcher,
      const std::vector<std::string>& test_names,
      const base::FilePath& output_file,
      const std::string& output,
      int exit_code,
      bool was_timeout,
      std::vector<std::string>* tests_to_relaunch) {
    std::vector<TestResult> test_results;
    bool crashed = false;
    bool have_test_results =
        ProcessGTestOutput(output_file, &test_results, &crashed);

    bool called_any_callback = false;

    if (have_test_results) {
      // TODO(phajdan.jr): Check for duplicates and mismatches between
      // the results we got from XML file and tests we intended to run.
      std::map<std::string, TestResult> results_map;
      for (size_t i = 0; i < test_results.size(); i++)
        results_map[test_results[i].full_name] = test_results[i];

      bool had_interrupted_test = false;

      // Results to be reported back to the test launcher.
      std::vector<TestResult> final_results;

      for (size_t i = 0; i < test_names.size(); i++) {
        if (ContainsKey(results_map, test_names[i])) {
          TestResult test_result = results_map[test_names[i]];
          if (test_result.status == TestResult::TEST_CRASH) {
            had_interrupted_test = true;

            if (was_timeout) {
              // Fix up the test status: we forcibly kill the child process
              // after the timeout, so from XML results it looks just like
              // a crash.
              test_result.status = TestResult::TEST_TIMEOUT;
            }
          } else if (test_result.status == TestResult::TEST_SUCCESS ||
                     test_result.status == TestResult::TEST_FAILURE) {
            // We run multiple tests in a batch with a timeout applied
            // to the entire batch. It is possible that with other tests
            // running quickly some tests take longer than the per-test timeout.
            // For consistent handling of tests independent of order and other
            // factors, mark them as timing out.
            if (test_result.elapsed_time >
                TestTimeouts::test_launcher_timeout()) {
              test_result.status = TestResult::TEST_TIMEOUT;
            }
          }
          test_result.output_snippet =
              GetTestOutputSnippet(test_result, output);
          final_results.push_back(test_result);
        } else if (had_interrupted_test) {
          tests_to_relaunch->push_back(test_names[i]);
        } else {
          // TODO(phajdan.jr): Explicitly pass the info that the test didn't
          // run for a mysterious reason.
          LOG(ERROR) << "no test result for " << test_names[i];
          TestResult test_result;
          test_result.full_name = test_names[i];
          test_result.status = TestResult::TEST_UNKNOWN;
          test_result.output_snippet =
              GetTestOutputSnippet(test_result, output);
          final_results.push_back(test_result);
        }
      }

      // TODO(phajdan.jr): Handle the case where processing XML output
      // indicates a crash but none of the test results is marked as crashing.

      if (final_results.empty())
        return false;

      bool has_non_success_test = false;
      for (size_t i = 0; i < final_results.size(); i++) {
        if (final_results[i].status != TestResult::TEST_SUCCESS) {
          has_non_success_test = true;
          break;
        }
      }

      if (!has_non_success_test && exit_code != 0) {
        // This is a bit surprising case: all tests are marked as successful,
        // but the exit code was not zero. This can happen e.g. under memory
        // tools that report leaks this way.

        if (final_results.size() == 1) {
          // Easy case. One test only so we know the non-zero exit code
          // was caused by that one test.
          final_results[0].status = TestResult::TEST_FAILURE_ON_EXIT;
        } else {
          // Harder case. Discard the results and request relaunching all
          // tests without batching. This will trigger above branch on
          // relaunch leading to more precise results.
          LOG(WARNING) << "Not sure which test caused non-zero exit code, "
                       << "relaunching all of them without batching.";

          for (size_t i = 0; i < final_results.size(); i++)
            tests_to_relaunch->push_back(final_results[i].full_name);

          return false;
        }
      }

      for (size_t i = 0; i < final_results.size(); i++) {
        // Fix the output snippet after possible changes to the test result.
        final_results[i].output_snippet =
            GetTestOutputSnippet(final_results[i], output);
        test_launcher->OnTestFinished(final_results[i]);
        called_any_callback = true;
      }
    } else {
      fprintf(stdout,
              "Failed to get out-of-band test success data, "
              "dumping full stdio below:\n%s\n",
              output.c_str());
      fflush(stdout);

      // We do not have reliable details about test results (parsing test
      // stdout is known to be unreliable), apply the executable exit code
      // to all tests.
      // TODO(phajdan.jr): Be smarter about this, e.g. retry each test
      // individually.
      for (size_t i = 0; i < test_names.size(); i++) {
        TestResult test_result;
        test_result.full_name = test_names[i];
        test_result.status = TestResult::TEST_UNKNOWN;
        test_launcher->OnTestFinished(test_result);
        called_any_callback = true;
      }
    }

    return called_any_callback;
  }

  ThreadChecker thread_checker_;

  // Maximum number of tests to run in a single batch.
  size_t batch_limit_;
};

bool GetSwitchValueAsInt(const std::string& switch_name, int* result) {
  if (!CommandLine::ForCurrentProcess()->HasSwitch(switch_name))
    return true;

  std::string switch_value =
      CommandLine::ForCurrentProcess()->GetSwitchValueASCII(switch_name);
  if (!StringToInt(switch_value, result) || *result < 1) {
    LOG(ERROR) << "Invalid value for " << switch_name << ": " << switch_value;
    return false;
  }

  return true;
}

int LaunchUnitTestsInternal(int argc,
                            char** argv,
                            const RunTestSuiteCallback& run_test_suite,
                            int default_jobs) {
  CommandLine::Init(argc, argv);

#if defined(OS_ANDROID)
  // We can't easily fork on Android, just run the test suite directly.
  return run_test_suite.Run();
#else
  bool force_single_process = false;
  if (CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kTestLauncherDebugLauncher)) {
    fprintf(stdout, "Forcing test launcher debugging mode.\n");
    fflush(stdout);
  } else {
    if (base::debug::BeingDebugged()) {
      fprintf(stdout,
              "Debugger detected, switching to single process mode.\n"
              "Pass --test-launcher-debug-launcher to debug the launcher "
              "itself.\n");
      fflush(stdout);
      force_single_process = true;
    }

    if (RunningOnValgrind()) {
      fprintf(stdout,
              "Valgrind detected, switching to single process mode.\n"
              "Pass --test-launcher-debug-launcher to valgrind the launcher "
              "itself.\n");
      fflush(stdout);
      force_single_process = true;
    }
  }

  if (CommandLine::ForCurrentProcess()->HasSwitch(kGTestHelpFlag) ||
      CommandLine::ForCurrentProcess()->HasSwitch(kGTestListTestsFlag) ||
      CommandLine::ForCurrentProcess()->HasSwitch(kSingleProcessTestsFlag) ||
      force_single_process) {
    return run_test_suite.Run();
  }
#endif

  if (CommandLine::ForCurrentProcess()->HasSwitch(kHelpFlag)) {
    PrintUsage();
    return 0;
  }

  base::TimeTicks start_time(base::TimeTicks::Now());

  testing::InitGoogleTest(&argc, argv);
  TestTimeouts::Initialize();

  int batch_limit = kDefaultTestBatchLimit;
  if (!GetSwitchValueAsInt(switches::kTestLauncherBatchLimit, &batch_limit))
    return 1;

  fprintf(stdout,
          "IMPORTANT DEBUGGING NOTE: batches of tests are run inside their\n"
          "own process. For debugging a test inside a debugger, use the\n"
          "--gtest_filter=<your_test_name> flag along with\n"
          "--single-process-tests.\n");
  fflush(stdout);

  MessageLoopForIO message_loop;

  UnitTestLauncherDelegate delegate(batch_limit);
  base::TestLauncher launcher(&delegate, default_jobs);
  bool success = launcher.Run(argc, argv);

  fprintf(stdout,
          "Tests took %" PRId64 " seconds.\n",
          (base::TimeTicks::Now() - start_time).InSeconds());
  fflush(stdout);

  return (success ? 0 : 1);
}

}  // namespace

int LaunchUnitTests(int argc,
                    char** argv,
                    const RunTestSuiteCallback& run_test_suite) {
  return LaunchUnitTestsInternal(
      argc, argv, run_test_suite, SysInfo::NumberOfProcessors());
}

int LaunchUnitTestsSerially(int argc,
                            char** argv,
                            const RunTestSuiteCallback& run_test_suite) {
  return LaunchUnitTestsInternal(argc, argv, run_test_suite, 1);
}

}  // namespace base
