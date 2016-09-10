#pragma once
#ifndef __XMLCHAR_H_0261C364_E6DD_4456_B6E1_09519AA9D67D__
#define __XMLCHAR_H_0261C364_E6DD_4456_B6E1_09519AA9D67D__

namespace xchar
{


template <typename XTCHAR>
inline static int x_istalnum(XTCHAR c) 
{
	return (c >= 0x30 && c <= 0x39) || (c >= 0x41 && c <= 0x5a) || (c >= 0x61 && c <= 0x7a) || (unsigned int)c > 0x7f;
}
	
template <typename XTCHAR>
inline static bool chartype_crlf(XTCHAR c) // character is return
{return (c == 10 || c == 13);}

template <typename XTCHAR>
inline static bool chartype_symbol(XTCHAR c) //Character is alphanumeric, -or- '_', -or- ':', -or- '-', -or- '.'.
{ return x_istalnum(c) || c == 95 || c == 58 || c == 45 || c == 46; }

template <typename XTCHAR>
inline static bool chartype_space(XTCHAR c) //Character is greater than 0 or character is less than exclamation.
{return (c == 9 || c == 10 || c== 13 || c == 32);}

template <typename XTCHAR>
inline static bool chartype_lt(XTCHAR c) //Character is '&lt;'.
{ return (c == 60); }

template <typename XTCHAR>
inline static bool chartype_gt(XTCHAR c) //Character is '&gt;'.
{ return (c == 62); }

template <typename XTCHAR>
inline static bool chartype_close(XTCHAR c) //Character is '/'.
{ return (c == 47); }

template <typename XTCHAR>
inline static bool chartype_equals(XTCHAR c) //Character is '='.
{ return (c == 61); }

template <typename XTCHAR>
inline static bool chartype_special(XTCHAR c) //Character is '!'.
{ return (c == 33); }

template <typename XTCHAR>
inline static bool chartype_pi(XTCHAR c) //Character is '?'.
{ return (c == 63); }

template <typename XTCHAR>
inline static bool chartype_dash(XTCHAR c) //Character is '-'.
{ return ( c== 45); }

template <typename XTCHAR>
inline static bool chartype_quote(XTCHAR c) //Character is &quot;&lsquo;&quot; -or- &lsquo;&quot;&lsquo;.
{ return (c == 34 || c == 39); }

template <typename XTCHAR>
inline static bool chartype_lbracket(XTCHAR c) //Character is '['.
{ return (c == 91); }

template <typename XTCHAR>
inline static bool chartype_rbracket(XTCHAR c) //Character is ']'.
{ return (c == 93); }

FILE* xfopen(const char* filename, const char* mode)
{
	return fopen(filename, mode);
}
#if defined(OS_WIN)
FILE* xfopen(const wchar_t* filename, const wchar_t* mode)
{
	return _wfopen(filename, mode);
}
#endif



}

#endif//__XMLCHAR_H_0261C364_E6DD_4456_B6E1_09519AA9D67D__
