#ifndef __ARCHIVES_STR_H_C1F2EE88_5119_4A06_A22B_53B4BD43E7F3__
#define __ARCHIVES_STR_H_C1F2EE88_5119_4A06_A22B_53B4BD43E7F3__

#include "./srlz_base.h"
#include <rv/str_conv.h>
#include <string>

NS_SERIALIZATION_LITE_BEGIN

//////////////////////////////////////////////////////////////////////////
// class iarchive_str
template <class CharType>
class iarchive_str : public iarchive_base< iarchive_str<CharType> >
{
public:
	typedef std::basic_string<CharType> StringType;
	inline iarchive_str(StringType const& str)
	: str_ref(str)
	{
	}
	inline StringType const& value() const
	{
		return str_ref;
	}
private:
	StringType const& str_ref;
};

//////////////////////////////////////////////////////////////////////////
// class oarchive_str
template <class CharType>
class oarchive_str : public oarchive_base< oarchive_str<CharType> >
{
public:
	typedef std::basic_string<CharType> StringType;
	inline oarchive_str(StringType& str)
	: str_ref(str)
	{
	}
	inline StringType& value()
	{
		return str_ref;
	}
private:
	StringType& str_ref;
};

//////////////////////////////////////////////////////////////////////////
//

//////////////////////////////////////////////////////////////////////////
// for string
template <class CharType>
inline void serialize_load(iarchive_str<CharType> const& ar, std::basic_string<CharType>& t)
{
	t = ar.value();
}

template <class CharType>
inline void serialize_save(oarchive_str<CharType>& ar, std::basic_string<CharType> const& t)
{
	ar.value() = t;
}

//////////////////////////////////////////////////////////////////////////
// for char
template <class CharType>
inline void serialize_load(const iarchive_str<CharType>& ar, char& t)
{
	int i;
	ar >> i;
	t = i;
}

template <class CharType>
inline void serialize_save(oarchive_str<CharType>& ar, const char& t)
{
	ar << static_cast<int>(t);
}

//////////////////////////////////////////////////////////////////////////
// for unsigned char
template <class CharType>
inline void serialize_load(const iarchive_str<CharType>& ar, unsigned char& t)
{
	unsigned int i;
	ar >> i;
	t = i;
}

template <class CharType>
inline void serialize_save(oarchive_str<CharType>& ar, const unsigned char& t)
{
	ar << static_cast<unsigned int>(t);
}

//////////////////////////////////////////////////////////////////////////
// macros for other basic types
template <typename CharType, typename IntType>
inline void string2integer(std::basic_string<CharType> const& s, IntType& t)
{
	const CharType* begin = s.data();
	const CharType* end = begin + s.length();
	tcfl::easy_atoi(begin, t, end);
}

template <typename CharType, typename IntType, size_t base, size_t digit>
inline void integer2string(std::basic_string<CharType>& s, IntType const&t)
{
	CharType buf[digit];
	size_t ret = tcfl::easy_itoa(t, buf, digit, base);
	s.assign(buf, ret);
}

template <typename CharType, typename FloatType>
inline void string2flaot(std::basic_string<CharType> const& s, FloatType& t)
{
	const CharType* begin = s.data();
	const CharType* end = begin + s.length();
	tcfl::easy_atof(begin, t, end);
}

template <typename CharType, typename FloatType>
inline void float2string(std::basic_string<CharType>& s, FloatType const&t)
{
	const size_t digit = 32;
	CharType buf[digit];
	size_t ret = tcfl::easy_ftoa(t, buf, digit, base);
	if (ret > digit)
	{
		s.resize(ret);
		tcfl::easy_ftoa(t, &s[0], ret);
	}
	else
		s.assign(buf, ret);
}

template <typename CharType, typename IntType>
inline void integer2hex(std::basic_string<CharType>& s, IntType const&t)
{
	CharType buf[sizeof(IntType) * 2];
	size_t ret = tcfl::easy_itoa(t, buf, sizeof(IntType) * 2, 16);
	const static CharType prefix[2] = {48, 120};
	s.reserve(sizeof(IntType) * 2 + 2);
	s.assign(prefix, 2);
	s.append(sizeof(IntType) * 2 - ret, (CharType)48);
	s.append(buf, ret);
}

#define STRING_TRANSLATE_INTEGER_TYPE(type, base, digit) \
template <class CharType>\
inline void serialize_load(iarchive_str<CharType> const& ar, type& t)\
{\
	string2integer<CharType, type>(ar.value(), t);\
}\
template <class CharType>\
inline void serialize_save(oarchive_str<CharType>& ar, const type& t)\
{\
	integer2string<CharType, type, base, digit>(ar.value(), t);\
}\

#define STRING_TRANSLATE_FLOAT_TYPE(type) \
template <class CharType>\
inline void serialize_load(iarchive_str<CharType> const& ar, type& t)\
{\
	string2flaot<CharType, type>(ar.value(), t);\
}\
template <class CharType>\
inline void serialize_save(oarchive_str<CharType>& ar, const type& t)\
{\
	float2string(ar.value(), t)\
}\

//////////////////////////////////////////////////////////////////////////
// for other basic types

//STRING_TRANSLATE_BASIC_TYPE(char)
//STRING_TRANSLATE_BASIC_TYPE(unsigned char)
STRING_TRANSLATE_INTEGER_TYPE(short, 10, 8)
STRING_TRANSLATE_INTEGER_TYPE(unsigned short, 10, 8)
STRING_TRANSLATE_INTEGER_TYPE(long, 10, 12)
STRING_TRANSLATE_INTEGER_TYPE(unsigned long, 10, 12)
STRING_TRANSLATE_INTEGER_TYPE(long long, 10, 24)
STRING_TRANSLATE_INTEGER_TYPE(unsigned long long, 10, 24)
STRING_TRANSLATE_INTEGER_TYPE(int, 10, 12)
STRING_TRANSLATE_INTEGER_TYPE(unsigned int, 10, 12)

STRING_TRANSLATE_FLOAT_TYPE(float)
STRING_TRANSLATE_FLOAT_TYPE(double)
STRING_TRANSLATE_FLOAT_TYPE(long double)

STRING_TRANSLATE_INTEGER_TYPE(bool, 10, 8)

//////////////////////////////////////////////////////////////////////////
// for void*
template <class CharType>
inline void serialize_load(const iarchive_str<CharType>& ar, void* t)
{
	string2integer<CharType, void*>(ar.value(), t);
}

template <class CharType>
inline void serialize_save(oarchive_str<CharType>& ar, void* const& t)
{
	integer2hex<CharType, void*>(ar.value(), t);
}


#undef STRING_TRANSLATE_INTEGER_TYPE
#undef STRING_TRANSLATE_FLOAT_TYPE


NS_SERIALIZATION_LITE_END


#endif//__ARCHIVES_STR_H_C1F2EE88_5119_4A06_A22B_53B4BD43E7F3__
