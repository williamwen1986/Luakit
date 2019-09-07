// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/business_client_main_parts.h"

namespace content {

int BusinessMainParts::PreCreateThreads() {
  return 0;
}

bool BusinessMainParts::MainMessageLoopRun(int* result_code) {
  return false;
}

}  // namespace content
