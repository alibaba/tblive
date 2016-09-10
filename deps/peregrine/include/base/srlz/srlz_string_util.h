#ifndef __SRLZ_STRING_UTIL_H__
#define __SRLZ_STRING_UTIL_H__

#include "base/strings/utf_string_conversions.h"

#ifdef OS_WIN
#include <comutil.h>
#include <atlstr.h>
#endif

namespace srlz { namespace internal {
    
    // suppout utf-8
    inline std::string StringNameConvert( const char* name ) {
        return name;
    }
    
    inline std::string StringNameConvert( const wchar_t* name ) {
        return base::WideToUTF8( name );
    }
    
    template<typename tchar>
    struct StringConvert;
    
    template<>
    struct StringConvert<char>
    {
        static std::string invoke( std::string const& s )
        {
            return s;
        }
    };
    
    template<>
    struct StringConvert<wchar_t>
    {
        static std::wstring invoke( std::string const& s )
        {
            return base::UTF8ToWide(s);
        }
    };
    
    // improve Object express when map<Key, Value> with Key is string type
    template<typename Key>
    struct map_key;
    
    template<>
    struct map_key<std::wstring>
    {
        static std::string Key2Str( std::wstring const& key )
        {
            return base::WideToUTF8(key);
        }
        
        static bool Str2Key( std::string const& str, std::wstring& key )
        {
            key = base::UTF8ToWide(str);
            return true;
        }
    };
    
    template<>
    struct map_key<std::string>
    {
        static std::string Key2Str( std::string const& key )
        {
            return key;
        }
        
        static bool Str2Key( std::string const& str, std::string& key )
        {
            key = str;
            return true;
        }
    };
    
#if defined(OS_WIN)
    template<>
    struct map_key<CString>
    {
        static std::string Key2Str( CString const& key )
        {
            return base::WideToUTF8(key.GetString());
        }
        
        static bool Str2Key( std::string const& str, CString& key )
        {
            key = base::UTF8ToWide(str).c_str();
            return true;
        }
    };
    
    
    template<>
    struct map_key<_bstr_t>
    {
        static std::string Key2Str( _bstr_t const& key )
        {
            return base::WideToUTF8((const wchar_t*)key);
        }
        
        static bool Str2Key( std::string const& str, _bstr_t& key )
        {
            key = base::UTF8ToWide(str).c_str();
            return true;
        }
    };
#endif
    
}  // namespace internal
}  // namespace srlz



#endif//__SRLZ_STRING_UTIL_H__
