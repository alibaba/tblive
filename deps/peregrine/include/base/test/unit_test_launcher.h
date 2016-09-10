// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TEST_UNIT_TEST_LAUNCHER_H_
#define BASE_TEST_UNIT_TEST_LAUNCHER_H_

#include "base/callback.h"

namespace base {

// Callback that runs a test suite and returns exit code.
typedef base::Callback<int(void)> RunTestSuiteCallback;

// Launches unit tests in given test suite. Returns exit code.
int LaunchUnitTests(int argc,
                    char** argv,
                    const RunTestSuiteCallback& run_test_suite);

}   // namespace base

#endif  // BASE_TEST_UNIT_TEST_LAUNCHER_H_
