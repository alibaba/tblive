// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_BASE_REGEX_UTIL_H_
#define NET_BASE_REGEX_UTIL_H_

#include "build/build_config.h"


#include <string>
#include <map>

#include "net/base/net_export.h"
#include "base/bind.h"
#include "base/callback.h"


namespace net {

// 正则表达式实现依赖icu
// see: http://userguide.icu-project.org/strings/regexp
	
// 将input中所有匹配到正则表达式的子串，传递到formatter格式化来替换，未匹配到的内原样拷贝
NET_EXPORT std::string RegexReplace( std::string const& input, std::string const& regex, base::Callback<std::string(std::string)> formatter );
	
// input字符串是否全匹配正则表达式
NET_EXPORT bool RegexMatch( std::string const& input, std::string const& regex );


}  // namespace net

#endif  // NET_BASE_REGEX_UTIL_H_
