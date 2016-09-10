// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TEST_PERFTIMER_H_
#define BASE_TEST_PERFTIMER_H_

#include "base/basictypes.h"
#include "base/time/time.h"

namespace base {
class FilePath;
}

// A simple wrapper around Now()
class PerfTimer {
 public:
  PerfTimer() {
    begin_ = base::TimeTicks::Now();
  }

  // Returns the time elapsed since object construction
  base::TimeDelta Elapsed() const {
    return base::TimeTicks::Now() - begin_;
  }

 private:
  base::TimeTicks begin_;

  DISALLOW_COPY_AND_ASSIGN(PerfTimer);
};

#endif  // BASE_TEST_PERFTIMER_H_
