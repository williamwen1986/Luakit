// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_BROWSER_SHUTDOWN_H_
#define CONTENT_PUBLIC_BROWSER_BROWSER_SHUTDOWN_H_
#pragma once

#include "common/content_export.h"

// This can be used for as-fast-as-possible shutdown, in cases where
// time for shutdown is limited and we just need to write out as much
// data as possible before our time runs out.
//
// This causes the shutdown sequence embodied by
// BusinessMainParts::PostMainMessageLoopRun through
// BusinessMainParts::PostDestroyThreads to occur, i.e. we pretend the
// message loop finished, all threads are stopped in sequence and then
// PostDestroyThreads is called.
//
// As this violates the normal order of shutdown, likely leaving the
// process in a bad state, the last thing this function does is
// terminate the process (right after calling
// BusinessMainParts::PostDestroyThreads).
CONTENT_EXPORT void ImmediateShutdownAndExitProcess();

#endif  // CONTENT_PUBLIC_BROWSER_BROWSER_SHUTDOWN_H_
