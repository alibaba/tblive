// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_BASE_ZLIB_UTIL_H_
#define NET_BASE_ZLIB_UTIL_H_

#include <string>

#include "net/base/net_export.h"


namespace net {
	
// 压缩级别
enum COMPRESS_LEVEL {
	CL_NO_COMPRESSION = 0,
	CL_BEST_SPEED = 1,
	CL_BEST_COMPRESSION = 9,
	
	CL_DEFAULT_COMPRESSION = -1
};
	
// 压缩
NET_EXPORT bool Compress(const std::string& src, std::string & dest, int level = CL_DEFAULT_COMPRESSION);

// 解压
NET_EXPORT bool UnCompress(const std::string& src, int destLen, std::string & dest);
	
}  // namespace net

#endif  // NET_BASE_ZLIB_UTIL_H_
