// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_STRINGS_GBK_STRING_CONVERSIONS_H_
#define BASE_STRINGS_GBK_STRING_CONVERSIONS_H_

#include <string>
#include "base/i18n/icu_string_conversions.h"

namespace base {

inline bool WideToGBK(const wchar_t* src, size_t src_len, std::string* output) 
{
    if ( !output ) 
    {
        return false;
    }

    return base::WideToCodepage( std::wstring(src, src_len), "GBK", base::OnStringConversionError::FAIL, output );
}

inline std::string WideToGBK(const std::wstring& wide) 
{
    std::string strGbk;
    bool success = base::WideToCodepage( wide, "GBK", base::OnStringConversionError::FAIL, &strGbk );
    if ( success ) {
        return strGbk;
    }

    return "";
}

inline bool GBKToWide(const char* src, size_t src_len, std::wstring* output) 
{
    if ( !output ) 
    {
        return false;
    }

    return base::CodepageToWide( std::string(src, src_len), "GBK", base::OnStringConversionError::FAIL, output );
}

inline std::wstring GBKToWide(const std::string& strGbk) 
{
    std::wstring wide;
    bool success = base::CodepageToWide( strGbk, "GBK", base::OnStringConversionError::FAIL, &wide );
    if ( success ) {
        return wide;
    }

    return L"";
}

}  // namespace base


#endif  // BASE_STRINGS_GBK_STRING_CONVERSIONS_H_
