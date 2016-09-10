// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ASYCLOGGING_H_
#define BASE_ASYCLOGGING_H_

#include <cassert>
#include <string>
#include <cstring>

#include "base/base_export.h"
#include "base/basictypes.h"
#include "build/build_config.h"
#include "base/files/file_path.h"
#include "base/logging.h"

BASE_EXPORT void InitAsycLoggingHandler(const base::FilePath &settingPath, const base::FilePath &logFilePath);
BASE_EXPORT void UninitAsycLoggingHandler();

#endif
