
#ifndef __STR_CONV_H_C98AC0D7_6DBF_4a59_81CB_345963291831__
#define __STR_CONV_H_C98AC0D7_6DBF_4a59_81CB_345963291831__
#include <math.h>

namespace tcfl{

template <typename CharType, typename IntType>
bool easy_atoi(const CharType* s, IntType& i, const CharType* end = NULL)
{
	IntType result = 0;

	if (s == NULL || s == end || *s == 0)
		return false;

	bool positive = true;
	IntType base = 10;

	const CharType* p = s;
	CharType c;
	bool succeed = false;
	do
	{
		c = *p;
		if (c >= 0x30 && c <= 0x39) // numbers
		{
			result = result * base + (c - 0x30);
		}
		else if(c >= 0x61 && c <= 0x66) // a - f
		{
			if (base != 16)
				break;
			result = result * base + (c - (0x61 - 0xa));
		}
		else if (c >= 0x41 && c <= 0x46) // A - F
		{
			if (base != 16)
				break;
			result = result * base + (c - (0x41 - 0xA));
		}
		else if (c == 0x58 || c == 0x78 || c == 0x24) // X or x or $
		{
			if (result || base != 10)
				break;
			base = 16;
		}
		else if (c == 0x20 || c == 0x9) // space or tab
			continue;
		else if (c == 0)
		{
			succeed = true;
			break;
		}
		else if (c == 0x2d) // minus
		{
			if (result)
				break;
			positive = false;
		}
		else if (c == 0x2b) // plus
		{
			if (result || !positive)
				break;
		}
		else
			break;
	}
	while (++p != end);

	if (succeed || p == end)
	{
		i = positive ? result : 0 - result;
		return true;
	}
	return false;
}

template <typename CharType, typename FloatType>
bool easy_atof(const CharType* s, FloatType& f, const CharType* end = NULL)
{
	FloatType result = 0;

	if (s == NULL || s == end || *s == 0)
		return false;

	bool positive = true;
	bool before_dot = true;
	bool succeed = false;
	FloatType base = 10.0;

	const CharType* p = s;
	CharType c;
	do
	{
		c = *p;
		if (c >= 0x30 && c <= 0x39) // numbers
		{
			if (before_dot)
				result = result * base + (c - 0x30);
			else
				result += (base /= 10.0) * (c - 0x30);
		}
		else if (c == 0x20 || c == 0x9) // space or tab
			continue;
		else if (c == 0x2e) // dot
		{
			if (!before_dot)
				break;
			before_dot = false;
			base = 1.0;
		}
		else if (c == 0)
		{
			succeed = true;
			break;
		}
		else if (c == 0x2d) // minus
		{
			if (result > 0 || result < 0)
				break;
			positive = false;
		}
		else if (c == 0x2b) // plus
		{
			if (result > 0 || result < 0 || !positive)
				break;
		}
		else
			break;
	} while (++p != end);

	if (succeed || p == end)
	{
		f = positive ? result : -result;
		return true;
	}
	return false;
}

template <typename CharType, typename IntType>
size_t easy_itoa(IntType i, CharType* s, size_t len, unsigned int base)
{
	if ((base != 10 && base != 16) || (i < 0 && base == 16))
		return 0;

	if (s == NULL)
		len = 0;

	size_t result = 0;
	IntType val;

	// if negative
	if (i < 0)
	{
		val = 0 - i;
		if (len > 1)
		{
			*s++ = 0x2d; // minus
			--len;
		}
		++result;
	}
	else
		val = i;
	IntType digival;
	CharType* p = s;
	CharType* end = s + len;

	do
	{
		++ result;
		digival = val % base;
		val /= base;
		if (p != end)
		{
			CharType digiChar = static_cast<CharType>(digival > 9 ? digival + (0x61 - 0xa) : digival + 0x30);
			*p++ = digiChar;
		}
	} while (val);

	// reverse string
	size_t real_len = p - s;  // exact length
	CharType* mid = s + real_len / 2;
	CharType* left = s;
	CharType* right = p - 1;
	while (left < mid)
	{
		CharType t = *left;
		*left = *right;
		*right = t;
		++left;
		--right;
	}
	return result;
}

template <typename CharType, typename IntType>
size_t easy_utoa(IntType i, CharType* s, size_t len, unsigned int base)
{
    if ((base != 10 && base != 16))
        return 0;
    
    if (s == NULL)
        len = 0;
    
    size_t result = 0;
    IntType val;
    
    // if negative
    val = i;
    
    IntType digival;
    CharType* p = s;
    CharType* end = s + len;
    
    do
    {
        ++ result;
        digival = val % base;
        val /= base;
        if (p != end)
        {
            CharType digiChar = static_cast<CharType>(digival > 9 ? digival + (0x61 - 0xa) : digival + 0x30);
            *p++ = digiChar;
        }
    } while (val);
    
    // reverse string
    size_t real_len = p - s;  // exact length
    CharType* mid = s + real_len / 2;
    CharType* left = s;
    CharType* right = p - 1;
    while (left < mid)
    {
        CharType t = *left;
        *left = *right;
        *right = t;
        ++left;
        --right;
    }
    return result;
}
    
#ifdef _WIN32
    inline float  easy_modf(float __x, float* __y)             {return modf(__x, __y);}
    inline double easy_modf(double __x, double* __y) {return modf(__x, __y);}
    inline double easy_modf(long double __x, long double* __y) {return modf(__x, __y);}
#else
    inline float  easy_modf(float __x, float* __y)             {return modff(__x, __y);}
    inline double easy_modf(double __x, double* __y) {return modf(__x, __y);}
    inline double easy_modf(long double __x, long double* __y) {return modfl(__x, __y);}
#endif

template <typename CharType, typename FloatType>
size_t easy_ftoa(FloatType f, CharType* s, size_t len)
{
	if (s == NULL)
		len = 0;

	FloatType fractional;
	FloatType integer;
	fractional = easy_modf(f, &integer);
	if (fractional < 0)
		fractional = 0 - fractional;
	size_t result = easy_itoa((long)integer, s, len, 10);
	s += result;

	const static long digi = 1000000000;
	long lfra = long(fractional * digi);
	if (lfra != 0)
	{
		if (len > result)
			*s++ = 0x2e; // dot
		++result;

		for (long _div = digi / 10; _div && lfra / _div == 0; _div /= 10)
		{
			if (len > result)
				*s++ = 0x30; //'0'
			++result;
		}
		while (lfra % 10 == 0)
			lfra /= 10;
		result += easy_itoa(lfra, s, len > result ? len - result : 0, 10);
	}
	return result;
}

inline int safe_wtoi(const wchar_t *str)
{
	if(str == NULL)
	{
		return 0;
	}
	return (int)wcstol(str, 0, 10);
}

inline long safe_wtol(const wchar_t *str)
{
	if(str == NULL)
	{
		return 0;
	}
	return wcstol(str, 0, 10);
}

inline long long safe_wtoll(const wchar_t *str)
{
	if(str == NULL)
	{
		return 0;
	}
#if defined(OS_WIN)
	return _wcstoi64(str, 0, 10);
#else
	return wcstoll(str, 0, 10);
#endif
}

inline unsigned long long safe_wtoull(const wchar_t *str)
{
	if(str == NULL)
	{
		return 0;
	}
#if defined(OS_WIN)
	return _wcstoui64(str, 0, 10);
#else
	return wcstoull(str, 0, 10);
#endif
}

inline int safe_atoi(const char *str)
{
	if(str == NULL)
	{
		return 0;
	}
	return atoi(str);
}

inline long safe_atol(const char *str)
{
	if(str == NULL)
	{
		return 0;
	}
	return atol(str);
}

}
#endif//__STR_CONV_H_C98AC0D7_6DBF_4a59_81CB_345963291831__
