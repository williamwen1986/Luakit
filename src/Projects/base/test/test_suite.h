// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TEST_TEST_SUITE_H_
#define BASE_TEST_TEST_SUITE_H_

// Defines a basic test suite framework for running gtest based tests.  You can
// instantiate this class in your main function and call its Run method to run
// any gtest based tests that are linked into your executable.

#include <string>

#include "base/at_exit.h"
#include "base/memory/scoped_ptr.h"

namespace testing {
class TestInfo;
}

namespace base {

// Instantiates TestSuite, runs it and returns exit code.
int RunUnitTestsUsingBaseTestSuite(int argc, char **argv);

class TestSuite {
 public:
  // Match function used by the GetTestCount method.
  typedef bool (*TestMatch)(const testing::TestInfo&);

  TestSuite(int argc, char** argv);
  virtual ~TestSuite();

  // Returns true if the test is marked as "MAYBE_".
  // When using different prefixes depending on platform, we use MAYBE_ and
  // preprocessor directives to replace MAYBE_ with the target prefix.
  static bool IsMarkedMaybe(const testing::TestInfo& test);

  void CatchMaybeTests();

  void ResetCommandLine();

  void AddTestLauncherResultPrinter();

  int Run();

 protected:
  // This constructor is only accessible to specialized test suite
  // implementations which need to control the creation of an AtExitManager
  // instance for the duration of the test.
  TestSuite(int argc, char** argv, bool create_at_exit_manager);

  // By default fatal log messages (e.g. from DCHECKs) result in error dialogs
  // which gum up buildbots. Use a minimalistic assert handler which just
  // terminates the process.
  static void UnitTestAssertHandler(const std::string& str);

  // Disable crash dialogs so that it doesn't gum up the buildbot
  virtual void SuppressErrorDialogs();

  // Override these for custom initialization and shutdown handling.  Use these
  // instead of putting complex code in your constructor/destructor.

  virtual void Initialize();
  virtual void Shutdown();

  // Make sure that we setup an AtExitManager so Singleton objects will be
  // destroyed.
  scoped_ptr<base::AtExitManager> at_exit_manager_;

 private:
  // Basic initialization for the test suite happens here.
  void PreInitialize(int argc, char** argv, bool create_at_exit_manager);

  bool initialized_command_line_;

  DISALLOW_COPY_AND_ASSIGN(TestSuite);
};

}  // namespace base

// TODO(brettw) remove this. This is a temporary hack to allow WebKit to compile
// until we can update it to use "base::" (preventing a two-sided patch).
using base::TestSuite;

#endif  // BASE_TEST_TEST_SUITE_H_
