// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TEST_TEST_LAUNCHER_H_
#define BASE_TEST_TEST_LAUNCHER_H_

#include <string>

#include "base/basictypes.h"
#include "base/callback_forward.h"
#include "base/compiler_specific.h"
#include "base/time/time.h"

class CommandLine;

namespace testing {
class TestCase;
class TestInfo;
}

namespace base {

struct LaunchOptions;

// Constants for GTest command-line flags.
extern const char kGTestFilterFlag[];
extern const char kGTestHelpFlag[];
extern const char kGTestListTestsFlag[];
extern const char kGTestRepeatFlag[];
extern const char kGTestRunDisabledTestsFlag[];
extern const char kGTestOutputFlag[];

// Structure containing result of a single test.
struct TestResult {
  enum Status {
    TEST_UNKNOWN,  // Status not set.
    TEST_SUCCESS,  // Test passed.
    TEST_FAILURE,  // Assertion failure (think EXPECT_TRUE, not DCHECK).
    TEST_TIMEOUT,  // Test timed out and was killed.
    TEST_CRASH,    // Test crashed (includes CHECK/DCHECK failures).
  };

  TestResult();

  std::string GetFullName() const { return test_case_name + "." + test_name; }

  // Returns true if the test has completed (i.e. the test binary exited
  // normally, possibly with an exit code indicating failure, but didn't crash
  // or time out in the middle of the test).
  bool completed() const {
    return status == TEST_SUCCESS || status == TEST_FAILURE;
  }

  // Name of the test case (before the dot, e.g. "A" for test "A.B").
  std::string test_case_name;

  // Name of the test (after the dot, e.g. "B" for test "A.B").
  std::string test_name;

  Status status;

  // Time it took to run the test.
  base::TimeDelta elapsed_time;
};

// Interface for use with LaunchTests that abstracts away exact details
// which tests and how are run.
class TestLauncherDelegate {
 public:
  // Called to get a test name for filtering purposes. Usually it's
  // test case's name and test's name joined by a dot (e.g.
  // "TestCaseName.TestName").
  // TODO(phajdan.jr): Remove after transitioning away from run_test_cases.py,
  // http://crbug.com/236893 .
  virtual std::string GetTestNameForFiltering(
      const testing::TestCase* test_case,
      const testing::TestInfo* test_info) = 0;

  // Called before a test is considered for running. If it returns false,
  // the test is not run. If it returns true, the test will be run provided
  // it is part of the current shard.
  virtual bool ShouldRunTest(const testing::TestCase* test_case,
                             const testing::TestInfo* test_info) = 0;

  // Called to make the delegate run specified test. After the delegate
  // finishes running the test (can do so asynchronously and out-of-order)
  // it must call |callback| regardless of test success.
  typedef base::Callback<void(const TestResult& result)> TestResultCallback;
  virtual void RunTest(const testing::TestCase* test_case,
                       const testing::TestInfo* test_info,
                       const TestResultCallback& callback) = 0;

  // If the delegate is running tests asynchronously, it must finish
  // running all pending tests and call their callbacks before returning
  // from this method.
  virtual void RunRemainingTests() = 0;

 protected:
  virtual ~TestLauncherDelegate();
};

// If |result| is not successful, prints that test's failure message
// (extracted from |full_output|) to stdout.
void PrintTestOutputSnippetOnFailure(const TestResult& result,
                                     const std::string& full_output);

// Launches a child process (assumed to be gtest-based binary)
// using |command_line|. If |wrapper| is not empty, it is prepended
// to the final command line. If the child process is still running
// after |timeout|, it is terminated and |*was_timeout| is set to true.
int LaunchChildGTestProcess(const CommandLine& command_line,
                            const std::string& wrapper,
                            base::TimeDelta timeout,
                            bool* was_timeout);

// Returns command line command line after gtest-specific processing
// and applying |wrapper|.
CommandLine PrepareCommandLineForGTest(const CommandLine& command_line,
                                       const std::string& wrapper);

// Launches a child process using |command_line|. If the child process is still
// running after |timeout|, it is terminated and |*was_timeout| is set to true.
// Returns exit code of the process.
int LaunchChildTestProcessWithOptions(const CommandLine& command_line,
                                      const LaunchOptions& options,
                                      base::TimeDelta timeout,
                                      bool* was_timeout);

// Launches GTest-based tests from the current executable
// using |launcher_delegate|.
int LaunchTests(TestLauncherDelegate* launcher_delegate,
                int argc,
                char** argv) WARN_UNUSED_RESULT;

}  // namespace base

#endif  // BASE_TEST_TEST_LAUNCHER_H_
