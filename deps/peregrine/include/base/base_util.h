// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_UTIL_H_
#define BASE_UTIL_H_

#include "base/base_export.h"
#include "base/i18n/icu_util.h"
#include "base/message_loop/message_loop.h"

#define ASSERT_IN_UITHREAD assert(base::MessageLoop::current() && base::MessageLoop::current()->type() == base::MessageLoop::TYPE_UI)

namespace base {
  // Must be called on main thread
  BASE_EXPORT bool InitUIMessageLoop();

  BASE_EXPORT base::MessageLoop* GetUIMessageLoop();

}  // namespace base

#endif  // BASE_UTIL_H_
