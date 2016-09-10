#pragma once


/*
 add by zhenshan: 2015-4-2
 almost same interface with ATL::CStringT, implement with pure C++ for portable purpose.
 */

#include <string>
#include <exception>
#include <algorithm>
#include <ostream>

#include "base/strings/sys_string_conversions.h"
#include "base/prg.h"
#include "base/asyclogginghandler.h"
#include "base/strings/stringprintf.h"
#include "base/namespace.h"

// srlz for PrgString
#include "base/srlz/archive_xparam.h"
#include "base/srlz/archive_json.h"
#include "base/srlz/archive_rapidjson.h"
#include "base/srlz/archive_rapidxml.h"

#include <type_traits>
#include "base/srlz/template_util.h"


BEGIN_NAMESPACE(prg)

namespace wwinternal
{
  template<typename tchar>
  struct context
  {
    static const char Space = ' ';
    
    static const char Lower_A = 'a';
    static const char Lower_Z = 'z';
    static const char Upper_A = 'A';
    static const char Upper_Z = 'Z';
  };
  
  template<>
  struct context<wchar_t>
  {
    static const wchar_t Space = L' ';
    
    static const wchar_t Lower_A = L'a';
    static const wchar_t Lower_Z = L'z';
    static const wchar_t Upper_A = L'A';
    static const wchar_t Upper_Z = L'Z';
  };
  
}// namespace wwinternal

template<typename tchar>
class PrgStringT
{
public:
  typedef std::basic_string<tchar, std::char_traits<tchar>, std::allocator<tchar> > tstring_type;
  
public:
  PrgStringT() {}
  
  PrgStringT( const PrgStringT& stringSrc ) : src_(stringSrc.src_)
  {
  }
  
  PrgStringT( PrgStringT&& stringSrc ) : src_( std::move(stringSrc.src_) )
  {
  }
  
  PrgStringT( tchar ch, int nRepeat = 1 ) : src_(nRepeat, ch)
  {
  }
  
  // support std::basic_string
  PrgStringT(tstring_type const& tstr) : src_(tstr)
  {
  }
  
  PrgStringT(tstring_type&& tstr) : src_( std::move(tstr) )
  {
  }
  
  PrgStringT( const tchar* lpch, int nLength ) : src_(lpch, nLength)
  {
  }
  
  PrgStringT( const tchar* lpsz ) : src_(lpsz)
  {
  }
  
public:
  int GetLength() const
  {
    return (int)src_.size();
  }
  
  bool IsEmpty() const
  {
    return src_.empty();
  }
  
  void Empty()
  {
    src_.clear();
  }
  
  tchar GetAt( int nIndex ) const
  {
    check_range(nIndex);
    
    return src_.at( nIndex );
  }
  
  tchar operator []( int nIndex ) const
  {
    return GetAt(nIndex);
  }
  
  tchar& operator []( int nIndex )
  {
    check_range(nIndex);
    
    return src_[nIndex];
  }
  
  void SetAt( int nIndex, tchar ch )
  {
    check_range(nIndex);
    
    src_[nIndex] = ch;
  }
  
  operator const tchar* ( ) const
  {
    return src_.c_str();
  }
  
  const tchar* GetString( ) const
  {
    return src_.c_str();
  }
  
public:
  // Assignment/Concatenation
  PrgStringT& operator =( const PrgStringT& stringSrc )
  {
    if ( this == &stringSrc )
    {
      return *this;
    }
    
    src_ = stringSrc.src_;
    return *this;
  }
  
  PrgStringT& operator =( PrgStringT&& stringSrc )
  {
    src_ = std::move(stringSrc.src_);
    return *this;
  }
  
  PrgStringT& operator = ( tchar ch )
  {
    src_ = ch;
    return *this;
  }
  
  PrgStringT& operator =( const tchar* lpsz )
  {
    src_ = lpsz;
    return *this;
  }
  
  PrgStringT& operator +=( const PrgStringT& string )
  {
    src_ += string.src_;
    return *this;
  }
  
  PrgStringT& operator +=( tchar ch )
  {
    src_ += ch;
    return *this;
  }
  
  PrgStringT& operator +=( const tchar* lpsz )
  {
    src_ += lpsz;
    return *this;
  }
  
public:
  // Comparison
  int Compare( const tchar* lpsz ) const
  {
    return src_.compare( lpsz );
  }
  
  int CompareNoCase( const tchar* lpsz ) const
  {
    PrgStringT lhs( src_.c_str() );
    PrgStringT rhs( lpsz );
    
    lhs.MakeLower();
    rhs.MakeLower();
    
    return lhs.Compare( rhs );
  }
  
  // Extraction
  PrgStringT Mid( int nFirst ) const
  {
    if ( nFirst > (int)src_.size() )
    {
        return PrgStringT();
    }
      
    return PrgStringT( src_.c_str() + nFirst );
  }
  
  PrgStringT Mid( int nFirst, int nCount ) const
  {
    if ( nFirst > (int)src_.size() )
    {
        return PrgStringT();
    }
      
    int len = nCount > ((int)src_.size() - nFirst) ? ((int)src_.size() - nFirst) : nCount;
    return PrgStringT( src_.c_str() + nFirst, len );
  }
  
  PrgStringT Left( int nCount ) const
  {
    if ( nCount > (int)src_.size() )
    {
        nCount = (int)src_.size();
    }
      
    return PrgStringT( src_.c_str(), nCount );
  }
  
  PrgStringT Right( int nCount ) const
  {
    int len = nCount > (int)src_.size() ? (int)src_.size() : nCount;
    return PrgStringT( src_.c_str() + (src_.size() - len), len );
  }
  
  PrgStringT Tokenize( const tchar* pszTokens, int& iStart ) const
  {
    PrgStringT tokens(pszTokens);
    
    // find first pos not in token
    auto beg = std::find_if( src_.begin() + iStart, src_.end(), [&tokens](tchar ch)->bool
                            {
                              return tokens.Find(ch) == -1;
                            });
    
    // nothing left
    if( beg == src_.end() )
    {
      iStart = -1;
      return PrgStringT();
    }
    
    // find first pos in token
    auto end = std::find_if( beg, src_.end(), [&tokens](tchar ch)->bool
                            {
                              return tokens.Find(ch) != -1;
                            });
    
    iStart = (int)std::distance( src_.begin(), end );
    return tstring_type(beg, end).c_str();
  }
  
public:
  // Other Conversions
  PrgStringT& MakeUpper()
  {
    tstring_type result;
    result.reserve( src_.size() );
    
    for (int i = 0; i < (int)src_.size(); ++i)
    {
      result.push_back( to_upper(src_[i]) );
    }
    std::swap( src_, result );
    return *this;
  }
  
  PrgStringT& MakeLower()
  {
    tstring_type result;
    result.reserve( src_.size() );
    for (int i = 0; i < (int)src_.size(); ++i)
    {
      result.push_back( to_lower(src_[i]) );
    }
    std::swap( src_, result );
    return *this;
  }
  
  PrgStringT& MakeReverse()
  {
    tstring_type result;
    result.reserve( src_.size() );
    
    int length = (int)src_.size();
    for (int i = 0; i < length; ++i)
    {
      result.push_back( src_[length-1 - i] );
    }
    std::swap( src_, result );
    return *this;
  }
  
  int Replace( tchar chOld, tchar chNew )
  {
    int count = 0;
    for (int i = 0; i < (int)src_.size(); ++i)
    {
      if ( src_[i] == chOld )
      {
        ++count;
        src_[i] = chNew;
      }
    }
    return count;
  }
  
  int Replace( const tchar* lpszOld, const tchar* lpszNew )
  {
    PrgStringT result;
    
    int count = 0;
    
    int len = (int) (tstring_type(lpszOld).size() );
    int start = 0;
    int find_pos = Find( lpszOld );
    
    while( find_pos != -1 )
    {
      // remain
      result += PrgStringT( src_.c_str() + start, find_pos - start );
      
      // replace
      ++count;
      result += lpszNew;
      start = find_pos + len;
      
      // next
      find_pos = Find( lpszOld, start );
    }
    
    if( start < (int)src_.size() )
    {
      result += PrgStringT( src_.c_str() + start );
    }
    
    std::swap( *this, result );
    
    return count;
  }
  
  int Remove( tchar ch )
  {
    auto fit = std::remove(src_.begin(), src_.end(), ch);
    int cnt = (int) (std::distance(fit, src_.end()) );
    
    src_.erase( fit, src_.end() );
    return cnt;
  }
  
  int Insert( int iIndex, const tchar* psz )
  {
    src_.insert( iIndex, psz );
    return (int)src_.size();
  }
  
  int Insert( int iIndex, tchar ch )
  {
    src_.insert( iIndex, 1, ch );
    return (int)src_.size();
  }
  
  int Delete( int iIndex, int nCount = 1 )
  {
    src_.erase( iIndex, nCount);
    return (int)src_.size();
  }
  
public:
  // Format
  template<typename A1>
  void Format(const tchar* lpszFormat, A1 const& a1)
  {
    FormatImpl(lpszFormat, TypeConvert(a1));
  }

  template<typename A1, typename A2>
  void Format(const tchar* lpszFormat, A1 const& a1, A2 const& a2)
  {
    FormatImpl(lpszFormat, TypeConvert(a1), TypeConvert(a2));
  }

  template<typename A1, typename A2, typename A3>
  void Format(const tchar* lpszFormat, A1 const& a1, A2 const& a2, A3 const& a3)
  {
    FormatImpl(lpszFormat, TypeConvert(a1), TypeConvert(a2), TypeConvert(a3));
  }

  template<typename A1, typename A2, typename A3, typename A4>
  void Format(const tchar* lpszFormat, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4)
  {
    FormatImpl(lpszFormat, TypeConvert(a1), TypeConvert(a2), TypeConvert(a3), TypeConvert(a4));
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5>
  void Format(const tchar* lpszFormat, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5)
  {
    FormatImpl(lpszFormat, TypeConvert(a1), TypeConvert(a2), TypeConvert(a3), TypeConvert(a4), TypeConvert(a5));
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
  void Format(const tchar* lpszFormat, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6)
  {
    FormatImpl(lpszFormat, TypeConvert(a1), TypeConvert(a2), TypeConvert(a3), TypeConvert(a4), TypeConvert(a5), TypeConvert(a6));
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
  void Format(const tchar* lpszFormat, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7)
  {
    FormatImpl(lpszFormat, TypeConvert(a1), TypeConvert(a2), TypeConvert(a3), TypeConvert(a4), TypeConvert(a5), TypeConvert(a6), TypeConvert(a7));
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
  void Format(const tchar* lpszFormat, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7, A8 const& a8)
  {
    FormatImpl(lpszFormat, TypeConvert(a1), TypeConvert(a2), TypeConvert(a3), TypeConvert(a4), TypeConvert(a5), TypeConvert(a6), TypeConvert(a7), TypeConvert(a8));
  }

  // AppendFormat
  template<typename A1>
  void AppendFormat(const tchar* lpszFormat, A1 const& a1)
  {
    AppendFormatImpl(lpszFormat, TypeConvert(a1));
  }

  template<typename A1, typename A2>
  void AppendFormat(const tchar* lpszFormat, A1 const& a1, A2 const& a2)
  {
    AppendFormatImpl(lpszFormat, TypeConvert(a1), TypeConvert(a2));
  }

  template<typename A1, typename A2, typename A3>
  void AppendFormat(const tchar* lpszFormat, A1 const& a1, A2 const& a2, A3 const& a3)
  {
    AppendFormatImpl(lpszFormat, TypeConvert(a1), TypeConvert(a2), TypeConvert(a3));
  }

  template<typename A1, typename A2, typename A3, typename A4>
  void AppendFormat(const tchar* lpszFormat, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4)
  {
    AppendFormatImpl(lpszFormat, TypeConvert(a1), TypeConvert(a2), TypeConvert(a3), TypeConvert(a4));
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5>
  void AppendFormat(const tchar* lpszFormat, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5)
  {
    AppendFormatImpl(lpszFormat, TypeConvert(a1), TypeConvert(a2), TypeConvert(a3), TypeConvert(a4), TypeConvert(a5));
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
  void AppendFormat(const tchar* lpszFormat, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6)
  {
    AppendFormatImpl(lpszFormat, TypeConvert(a1), TypeConvert(a2), TypeConvert(a3), TypeConvert(a4), TypeConvert(a5), TypeConvert(a6));
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
  void AppendFormat(const tchar* lpszFormat, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7)
  {
    AppendFormatImpl(lpszFormat, TypeConvert(a1), TypeConvert(a2), TypeConvert(a3), TypeConvert(a4), TypeConvert(a5), TypeConvert(a6), TypeConvert(a7));
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
  void AppendFormat(const tchar* lpszFormat, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7, A8 const& a8)
  {
    AppendFormatImpl(lpszFormat, TypeConvert(a1), TypeConvert(a2), TypeConvert(a3), TypeConvert(a4), TypeConvert(a5), TypeConvert(a6), TypeConvert(a7), TypeConvert(a8));
  }
  
private:
  // FormatImpl
  void FormatImpl(const tchar* lpszFormat, ...) {
    va_list ap;
    va_start(ap, lpszFormat);
    src_ = base::StringPrintV(lpszFormat, ap);
    va_end(ap);
  }
  
  // AppendFormatImpl
  void AppendFormatImpl( const tchar* lpszFormat, ...)
  {
    va_list ap;
    va_start(ap, lpszFormat);
    tstring_type tmp = base::StringPrintV(lpszFormat, ap);
    va_end(ap);
    
    (*this) += tmp.c_str();
  }
  
  // Convert type to primitive types which are supported by base::StringPrintV
  // Support: integral(include char), float, enum
  template<typename T>
  static typename srlz::internal::enable_if_c< std::is_enum<T>::value || std::is_integral<T>::value || std::is_floating_point<T>::value, T >::type TypeConvert( T const& t )
  {
    return t;
  }
	
  // const tchar*
  static const tchar* TypeConvert( const tchar* t )
  {
    return t;
  }
  
  static const tchar* TypeConvert( tstring_type const& t )
  {
    return t.c_str();
  }
  
  static const tchar* TypeConvert(PrgStringT const& t)
  {
    return t.GetString();
  }
  
public:
  PrgStringT& Trim( tchar chTarget = wwinternal::context<tchar>::Space )
  {
    int leftCnt = 0;
    for (int i = 0; i < (int)src_.size(); ++i)
    {
      if( src_[i] == chTarget )
      {
        ++leftCnt;
      }
      else
        break;
    }
    
    // empty
    if (leftCnt == (int)src_.size())
    {
      src_.clear();
      return *this;
    }
    
    int rightCnt = 0;
    for (int i = 0; i < (int)src_.size(); ++i)
    {
      if( src_[src_.size()-1 - i] == chTarget )
      {
        ++rightCnt;
      }
      else
        break;
    }
    
    tstring_type result( src_.c_str() + leftCnt, src_.size() - leftCnt - rightCnt );
    std::swap(src_, result);
    return *this;
  }
  
  PrgStringT& Trim( const tchar* pszTargets )
  {
    PrgStringT targets(pszTargets);
    
    int leftCnt = 0;
    for (int i = 0; i < (int)src_.size(); ++i)
    {
      if( targets.Find(src_[i]) != -1 )
      {
        ++leftCnt;
      }
      else
        break;
    }
    
    // empty
    if (leftCnt == (int)src_.size())
    {
      src_.clear();
      return *this;
    }
    
    int rightCnt = 0;
    for (int i = 0; i < (int)src_.size(); ++i)
    {
      if( targets.Find(src_[src_.size()-1 - i]) != -1 )
      {
        ++rightCnt;
      }
      else
        break;
    }
    
    tstring_type result( src_.c_str() + leftCnt, src_.size() - leftCnt - rightCnt );
    std::swap(src_, result);
    return *this;
  }
  
  PrgStringT& TrimLeft( tchar chTarget = wwinternal::context<tchar>::Space )
  {
    int leftCnt = 0;
    for (int i = 0; i < (int)src_.size(); ++i)
    {
      if( src_[i] == chTarget )
      {
        ++leftCnt;
      }
      else
        break;
    }
    
    tstring_type result( src_.c_str() + leftCnt );
    std::swap(src_, result);
    return *this;
  }
  
  PrgStringT& TrimLeft( const tchar* lpszTargets )
  {
    PrgStringT targets(lpszTargets);
    
    int leftCnt = 0;
    for (int i = 0; i < (int)src_.size(); ++i)
    {
      if( targets.Find(src_[i]) != -1 )
      {
        ++leftCnt;
      }
      else
        break;
    }
    
    tstring_type result( src_.c_str() + leftCnt );
    std::swap(src_, result);
    return *this;
  }
  
  PrgStringT& TrimRight( tchar chTarget = wwinternal::context<tchar>::Space )
  {
    int rightCnt = 0;
    for (int i = 0; i < (int)src_.size(); ++i)
    {
      if( src_[src_.size()-1 - i] == chTarget )
      {
        ++rightCnt;
      }
      else
        break;
    }
    
    tstring_type result( src_.c_str(), src_.size() - rightCnt );
    std::swap(src_, result);
    return *this;
  }
  
  PrgStringT& TrimRight( const tchar* lpszTargets )
  {
    PrgStringT targets(lpszTargets);
    
    int rightCnt = 0;
    for (int i = 0; i < (int)src_.size(); ++i)
    {
      if( targets.Find(src_[src_.size()-1 - i]) != -1 )
      {
        ++rightCnt;
      }
      else
        break;
    }
    
    tstring_type result( src_.c_str(), src_.size() - rightCnt );
    std::swap(src_, result);
    return *this;
  }
  
public:
  // Search
  int Find( tchar ch, int nStart = 0 ) const
  {
    typename tstring_type::size_type pos = src_.find( ch, nStart );
    if ( pos != tstring_type::npos )
    {
      return (int)pos;
    }
    return -1;
  }
  
  int Find( const tchar* lpszSub, int nStart = 0 ) const
  {
    typename tstring_type::size_type pos = src_.find( lpszSub, nStart );
    if ( pos != tstring_type::npos )
    {
      return (int)pos;
    }
    return -1;
  }
  
  int ReverseFind( tchar ch ) const
  {
    typename tstring_type::size_type pos = src_.find_last_of( ch );
    if ( pos != tstring_type::npos )
    {
      return (int)pos;
    }
    return -1;
  }
  
  int FindOneOf( const tchar* lpszCharSet ) const
  {
    typename tstring_type::size_type pos = src_.find_first_of( lpszCharSet );
    if ( pos != tstring_type::npos )
    {
      return (int)pos;
    }
    return -1;
  }
  
private:
  void check_range( int nIndex ) const
  {
    if( (int)src_.size() <= nIndex )
    {
      assert(false && "bad range");
      throw std::exception();
    }
  }
  
  tchar to_lower( tchar ch )
  {
    if( wwinternal::context<tchar>::Upper_A <= ch &&
       ch <= wwinternal::context<tchar>::Upper_Z )
    {
      return wwinternal::context<tchar>::Lower_A + (ch - wwinternal::context<tchar>::Upper_A);
    }
    return ch;
  }
  
  tchar to_upper( tchar ch )
  {
    if( wwinternal::context<tchar>::Lower_A <= ch &&
       ch <= wwinternal::context<tchar>::Lower_Z )
    {
      return wwinternal::context<tchar>::Upper_A + (ch - wwinternal::context<tchar>::Lower_A);
    }
    return ch;
  }
  
private:
  tstring_type src_;
};

template<typename tchar>
inline std::basic_ostream<tchar, std::char_traits<tchar> >& operator << (
                                                                         std::basic_ostream<tchar, std::char_traits<tchar> >& os,
                                                                         PrgStringT<tchar> const& str )
{
  os << str.GetString();
  return os;
}

// operator +
template<typename tchar>
inline PrgStringT<tchar> operator + ( const PrgStringT<tchar>& string1, const PrgStringT<tchar>& string2 )
{
  PrgStringT<tchar> result(string1);
  result += string2;
  return result;
}

template<typename tchar>
inline PrgStringT<tchar> operator + ( const PrgStringT<tchar>& string, tchar ch )
{
  PrgStringT<tchar> result(string);
  result += ch;
  return result;
}

template<typename tchar>
inline PrgStringT<tchar> operator + ( tchar ch, const PrgStringT<tchar>& string )
{
  PrgStringT<tchar> result(ch);
  result += string;
  return result;
}

template<typename tchar>
inline PrgStringT<tchar> operator + ( const PrgStringT<tchar>& string, const tchar* lpsz )
{
  PrgStringT<tchar> result(string);
  result += lpsz;
  return result;
}

template<typename tchar>
inline PrgStringT<tchar> operator +( const tchar* lpsz, const PrgStringT<tchar>& string )
{
  PrgStringT<tchar> result(lpsz);
  result += string;
  return result;
}

// operator ==
template<typename tchar>
inline bool operator ==( const PrgStringT<tchar>& s1, const PrgStringT<tchar>& s2 )
{
  std::basic_string<tchar> lhs( s1 );
  return lhs == (const tchar*)s2;
}

template<typename tchar>
inline bool operator ==( const PrgStringT<tchar>& s1, const tchar* s2 )
{
  std::basic_string<tchar> lhs( s1 );
  return lhs == s2;
}

template<typename tchar>
inline bool operator ==( const tchar* s1, const PrgStringT<tchar>& s2 )
{
  std::basic_string<tchar> lhs( s1 );
  return lhs == (const tchar*)s2;
}

// operator !=
template<typename tchar>
inline bool operator !=( const PrgStringT<tchar>& s1, const PrgStringT<tchar>& s2 )
{
  std::basic_string<tchar> lhs( s1 );
  return lhs != (const tchar*)s2;
}

template<typename tchar>
inline bool operator !=( const PrgStringT<tchar>& s1, const tchar* s2 )
{
  std::basic_string<tchar> lhs( s1 );
  return lhs != s2;
}

template<typename tchar>
inline bool operator !=( const tchar* s1, const PrgStringT<tchar>& s2 )
{
  std::basic_string<tchar> lhs( s1 );
  return lhs != (const tchar*)s2;
}

// operator <
template<typename tchar>
inline bool operator <( const PrgStringT<tchar>& s1, const PrgStringT<tchar>& s2 )
{
  std::basic_string<tchar> lhs( s1 );
  return lhs < (const tchar*)s2;
}

template<typename tchar>
inline bool operator <( const PrgStringT<tchar>& s1, const tchar* s2 )
{
  std::basic_string<tchar> lhs( s1 );
  return lhs < s2;
}

template<typename tchar>
inline bool operator <( const tchar* s1, const PrgStringT<tchar>& s2 )
{
  std::basic_string<tchar> lhs( s1 );
  return lhs < (const tchar*)s2;
}

// operator >
template<typename tchar>
inline bool operator >( const PrgStringT<tchar>& s1, const PrgStringT<tchar>& s2 )
{
  std::basic_string<tchar> lhs( s1 );
  return lhs > (const tchar*)s2;
}

template<typename tchar>
inline bool operator >( const PrgStringT<tchar>& s1, const tchar* s2 )
{
  std::basic_string<tchar> lhs( s1 );
  return lhs > s2;
}

template<typename tchar>
inline bool operator >( const tchar* s1, const PrgStringT<tchar>& s2 )
{
  std::basic_string<tchar> lhs( s1 );
  return lhs > (const tchar*)s2;
}

// operator <=
template<typename tchar>
inline bool operator <=( const PrgStringT<tchar>& s1, const PrgStringT<tchar>& s2 )
{
  std::basic_string<tchar> lhs( s1 );
  return lhs <= (const tchar*)s2;
}

template<typename tchar>
inline bool operator <=( const PrgStringT<tchar>& s1, const tchar* s2 )
{
  std::basic_string<tchar> lhs( s1 );
  return lhs <= s2;
}

template<typename tchar>
inline bool operator <=( const tchar* s1, const PrgStringT<tchar>& s2 )
{
  std::basic_string<tchar> lhs( s1 );
  return lhs <= (const tchar*)s2;
}

// operator >=
template<typename tchar>
inline bool operator >=( const PrgStringT<tchar>& s1, const PrgStringT<tchar>& s2 )
{
  std::basic_string<tchar> lhs( s1 );
  return lhs >= (const tchar*)s2;
}

template<typename tchar>
inline bool operator >=( const PrgStringT<tchar>& s1, const tchar* s2 )
{
  std::basic_string<tchar> lhs( s1 );
  return lhs >= s2;
}

template<typename tchar>
inline bool operator >=( const tchar* s1, const PrgStringT<tchar>& s2 )
{
  std::basic_string<tchar> lhs( s1 );
  return lhs >= (const tchar*)s2;
}

typedef PrgStringT<char>		PrgStringA;
typedef PrgStringT<wchar_t>	PrgStringW;

typedef PrgStringW			PrgString;


END_NAMESPACE()

// PrgString is used so frequently, so using name "PrgString" directly
using prg::PrgString;

// srlz
NS_SERIALIZATION_LITE_BEGIN

namespace internal {

template<>
struct map_key<PrgString>
{
  static std::string Key2Str( PrgString const& key )
  {
    return base::WideToUTF8(key.GetString());
  }

  static bool Str2Key( std::string const& str, PrgString& key )
  {
    key = base::UTF8ToWide(str).c_str();
    return true;
  }
};

}// namespace internal

// json
template <class CharType>
inline void serialize_load(iarchive_json const& ar, prg::PrgStringT<CharType>& t)
{
  typename prg::PrgStringT<CharType>::tstring_type str;
  serialize_load( ar, str );
  t = str;
}

template <class CharType>
inline void serialize_save(oarchive_json& ar, prg::PrgStringT<CharType> const& t)
{
  typename prg::PrgStringT<CharType>::tstring_type str = (const CharType*)t;
  serialize_save( ar, str );
}

// rapidjson
template <class CharType>
inline void serialize_load(iarchive_rapidjson const& ar, prg::PrgStringT<CharType>& t)
{
    typename prg::PrgStringT<CharType>::tstring_type str;
    serialize_load( ar, str );
    t = str;
}

template <class CharType>
inline void serialize_save(oarchive_rapidjson& ar, prg::PrgStringT<CharType> const& t)
{
    typename prg::PrgStringT<CharType>::tstring_type str = (const CharType*)t;
    serialize_save( ar, str );
}

// rapidxml
template <class CharType>
inline void serialize_load(iarchive_rapidxml<char> const& ar, prg::PrgStringT<CharType>& t)
{
    typename prg::PrgStringT<CharType>::tstring_type str;
    serialize_load( ar, str );
    t = str;
}

template <class CharType>
inline void serialize_save(oarchive_rapidxml<char>& ar, prg::PrgStringT<CharType> const& t)
{
    typename prg::PrgStringT<CharType>::tstring_type str = (const CharType*)t;
    serialize_save( ar, str );
}

// xparam
template <typename CharType>
inline void serialize_load(iarchive_xparam const& ar, prg::PrgStringT<CharType>& t)
{
  if (ar.exists())
  {
    CharType const* v = {0};
    size_t vlen = 0;
    xparam::XInResP inres;
    if (ar.value(v, &vlen, inres))
    {
      t = prg::PrgStringT<CharType>(v, (int)vlen);
    }
  }
}

template <typename CharType>
inline void serialize_save(oarchive_xparam& ar, prg::PrgStringT<CharType> const& t)
{
  ar.value((const CharType*)t, t.GetLength());
}


NS_SERIALIZATION_LITE_END
