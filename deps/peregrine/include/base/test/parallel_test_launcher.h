// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TEST_PARALLEL_TEST_LAUNCHER_H_
#define BASE_TEST_PARALLEL_TEST_LAUNCHER_H_

#include <map>
#include <string>

#include "base/callback.h"
#include "base/threading/sequenced_worker_pool.h"
#include "base/threading/thread_checker.h"
#include "base/time/time.h"
#include "base/timer/timer.h"

class CommandLine;

namespace base {

class SequencedWorkerPoolOwner;
class Timer;

// Launches child gtest process in parallel. Keeps track of running processes,
// prints a message in case no output is produced for a while.
class ParallelTestLauncher {
 public:
  // Constructor. |jobs| is the maximum number of tests launched in parallel.
  explicit ParallelTestLauncher(size_t jobs);
  ~ParallelTestLauncher();

  // Callback called after a child process finishes. First argument is the exit
  // code, second one is child process elapsed time, third one is true if
  // the child process was terminated because of a timeout, and fourth one
  // contains output of the child (stdout and stderr together).
  typedef Callback<void(int, const TimeDelta&, bool, const std::string&)>
      LaunchChildGTestProcessCallback;

  // Launches a child process (assumed to be gtest-based binary) using
  // |command_line|. If |wrapper| is not empty, it is prepended to the final
  // command line. If the child process is still running after |timeout|, it
  // is terminated. After the child process finishes |callback| is called
  // on the same thread this method was called.
  void LaunchChildGTestProcess(const CommandLine& command_line,
                               const std::string& wrapper,
                               base::TimeDelta timeout,
                               const LaunchChildGTestProcessCallback& callback);

  // Similar to above, but with processes sharing the same value of |token_name|
  // being serialized, with order matching order of calls of this method.
  void LaunchNamedSequencedChildGTestProcess(
      const std::string& token_name,
      const CommandLine& command_line,
      const std::string& wrapper,
      base::TimeDelta timeout,
      const LaunchChildGTestProcessCallback& callback);

  // Resets the output watchdog, indicating some test results have been printed
  // out. If a pause between the calls exceeds an internal treshold, a message
  // will be printed listing all child processes we're still waiting for.
  void ResetOutputWatchdog();

 private:
  void LaunchSequencedChildGTestProcess(
      SequencedWorkerPool::SequenceToken sequence_token,
      const CommandLine& command_line,
      const std::string& wrapper,
      base::TimeDelta timeout,
      const LaunchChildGTestProcessCallback& callback);

  // Called on a worker thread after a child process finishes.
  void OnLaunchTestProcessFinished(
      size_t sequence_number,
      const LaunchChildGTestProcessCallback& callback,
      int exit_code,
      const TimeDelta& elapsed_time,
      bool was_timeout,
      const std::string& output);

  // Called by the delay timer when no output was made for a while.
  void OnOutputTimeout();

  // Make sure we don't accidentally call the wrong methods e.g. on the worker
  // pool thread. With lots of callbacks used this is non-trivial.
  // Should be the first member so that it's destroyed last: when destroying
  // other members, especially the worker pool, we may check the code is running
  // on the correct thread.
  ThreadChecker thread_checker_;

  // Watchdog timer to make sure we do not go without output for too long.
  DelayTimer<ParallelTestLauncher> timer_;

  // Monotonically increasing sequence number to uniquely identify each
  // launched child process.
  size_t launch_sequence_number_;

  // Map of currently running child processes, keyed by the sequence number.
  typedef std::map<size_t, CommandLine> RunningProcessesMap;
  RunningProcessesMap running_processes_map_;

  // Worker pool used to launch processes in parallel.
  scoped_ptr<SequencedWorkerPoolOwner> worker_pool_owner_;

  DISALLOW_COPY_AND_ASSIGN(ParallelTestLauncher);
};

}  // namespace base

#endif  // BASE_TEST_PARALLEL_TEST_LAUNCHER_H_
