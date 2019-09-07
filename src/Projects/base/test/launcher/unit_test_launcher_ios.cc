// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/test/launcher/unit_test_launcher.h"

namespace base {

int LaunchUnitTests(int argc,
                    char** argv,
                    const RunTestSuiteCallback& run_test_suite) {
  // Stub implementation - iOS doesn't support features we need for
  // the full test launcher (e.g. process_util).
  return run_test_suite.Run();
}

}  // namespace base
