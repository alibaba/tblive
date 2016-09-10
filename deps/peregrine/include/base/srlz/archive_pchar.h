#ifndef __ARCHIVES_PCHAR_H_C1F2EE88_5119_4A06_A22B_53B4BD43E7F3__
#define __ARCHIVES_PCHAR_H_C1F2EE88_5119_4A06_A22B_53B4BD43E7F3__

#include "./srlz_base.h"
#include <base/str_conv.h>
#include <string>

NS_SERIALIZATION_LITE_BEGIN
#ifndef ZERO_TERM
#define ZERO_TERM ((size_t)-1)
#endif

template <typename CharType>
inline void normalize_len(CharType const* s, size_t& len)
{
	if (s == NULL)
		len = 0;
	else if (len == ZERO_TERM)
	{
		CharType const* eos = s;
		while( *eos++ );
		len =  eos - s - 1;
	}
}

//////////////////////////////////////////////////////////////////////////
// class iarchive_pchar
template <class CharType>
class iarchive_pchar : public iarchive_base< iarchive_pchar<CharType> >
{
public:
	inline iarchive_pchar(CharType const* s, size_t l) : pchar(s), len(l) {normalize_len(pchar, len);}
	template <typename IntType> inline void to_integer(IntType& i) const
	{
		tcfl::easy_atoi(pchar, i, pchar + len);
	}
	template <typename FloatType> inline void to_float(FloatType& f) const
	{
		tcfl::easy_atof(pchar, f, pchar + len);
	}
	CharType const* const pchar;
	size_t len;
};

//////////////////////////////////////////////////////////////////////////
// class oarchive_pchar
template <class CharType>
class oarchive_pchar : public oarchive_base< oarchive_pchar<CharType> >
{
public:
	inline oarchive_pchar() : pchar(NULL), len(0) {}
	inline ~oarchive_pchar() {delete[] pchar;}
	inline void realloc(size_t l)
	{
		delete[] pchar;
		len = l;
		pchar = new CharType[len];
	}
	template <typename IntType> inline void from_integer(IntType i)
	{
		realloc(sizeof(i) < 2 ? 8 : sizeof(i) * 4);
		len = tcfl::easy_itoa(i, pchar, len, 10);
		pchar[len] = 0;
	}
	template <typename FloatType> inline void from_float(FloatType f)
	{
		realloc(32);
		len = tcfl::easy_ftoa(f, pchar, len);
		pchar[len] = 0;
	}
	template <typename IntType> inline void hex_from_integer(IntType i)
	{
		CharType buf[sizeof(IntType) * 2];
		size_t ret = tcfl::easy_itoa(i, buf, sizeof(IntType) * 2, 16);
		realloc(sizeof(IntType) * 2 + 2 + 1);
		pchar[0] = 48;  // 0
		pchar[1] = 120; // x
		size_t n = 2;
		for (; n < sizeof(IntType) * 2 - ret + 2; ++ n)
			pchar[n] = 48;
		memcpy(pchar + n, buf, ret * sizeof(CharType));
		pchar[--len] = 0;
	}
	CharType* pchar;
	size_t len;
};

//////////////////////////////////////////////////////////////////////////
// for string
template <class CharType, typename _Traits, typename _Ax>
inline void serialize_load(iarchive_pchar<CharType> const& ar, std::basic_string<CharType, _Traits, _Ax>& t)
{
	if (ar.len)
		t.assign(ar.pchar, ar.len);
	else
		t.clear();
}

template <class CharType, typename _Traits, typename _Ax>
inline void serialize_save(oarchive_pchar<CharType>& ar, std::basic_string<CharType, _Traits, _Ax> const& t)
{
	ar.realloc(t.length() + 1);
	memcpy(ar.pchar, t.data(), sizeof(CharType) * t.length());
	ar.pchar[--ar.len] = 0;
}

//////////////////////////////////////////////////////////////////////////
// macros for other basic types
#define STRING_TRANSLATE_INTEGER_TYPE(type) \
template <class CharType>\
inline void serialize_load(iarchive_pchar<CharType> const& ar, type& t)\
{\
	ar.to_integer(t);\
}\
template <class CharType>\
inline void serialize_save(oarchive_pchar<CharType>& ar, const type& t)\
{\
	ar.from_integer(t);\
}\

#define STRING_TRANSLATE_FLOAT_TYPE(type) \
template <class CharType>\
inline void serialize_load(iarchive_pchar<CharType> const& ar, type& t)\
{\
	ar.to_float(t);\
}\
template <class CharType>\
inline void serialize_save(oarchive_pchar<CharType>& ar, const type& t)\
{\
	ar.from_float(t);\
}\

//////////////////////////////////////////////////////////////////////////
// for other basic types

STRING_TRANSLATE_INTEGER_TYPE(char)
STRING_TRANSLATE_INTEGER_TYPE(unsigned char)
STRING_TRANSLATE_INTEGER_TYPE(short)
STRING_TRANSLATE_INTEGER_TYPE(unsigned short)
STRING_TRANSLATE_INTEGER_TYPE(long)
STRING_TRANSLATE_INTEGER_TYPE(unsigned long)
STRING_TRANSLATE_INTEGER_TYPE(long long)
STRING_TRANSLATE_INTEGER_TYPE(unsigned long long)
STRING_TRANSLATE_INTEGER_TYPE(int)
STRING_TRANSLATE_INTEGER_TYPE(unsigned int)

STRING_TRANSLATE_FLOAT_TYPE(float)
STRING_TRANSLATE_FLOAT_TYPE(double)
STRING_TRANSLATE_FLOAT_TYPE(long double)


//////////////////////////////////////////////////////////////////////////
// for bool
template <class CharType>
inline void serialize_load(const iarchive_pchar<CharType>& ar, bool& t)
{
	int i;
	if (tcfl::easy_atoi(ar.pchar, i, ar.pchar + ar.len))
		t = i != 0;
}

template <class CharType>
inline void serialize_save(oarchive_pchar<CharType>& ar, bool const& t)
{
	int i = t == true;
	ar.from_integer(i);
}

//////////////////////////////////////////////////////////////////////////
// for char array
template <class CharType, size_t arrsize>
inline void serialize_load(const iarchive_pchar<CharType>& ar, CharType (&t)[arrsize])
{
	size_t copy_len = ar.len > arrsize - 1 ? arrsize - 1 : ar.len;
	memcpy(t, ar.pchar, copy_len * sizeof(CharType));
	t[copy_len] = 0;
}

template <class CharType, size_t arrsize>
inline void serialize_save(oarchive_pchar<CharType>& ar, CharType const (&t)[arrsize])
{
	ar.realloc(arrsize + 1);
	ar.pchar[arrsize] = 0;
	CharType * cp = ar.pchar;
	CharType const* src = t;
	while( (cp - ar.pchar < arrsize) && (*cp++ = *src++) )
		;
	ar.len = cp - ar.pchar - 1;
}


//////////////////////////////////////////////////////////////////////////
// for void*

template <size_t size> struct right_int_getter;
template <> struct right_int_getter<4> {typedef unsigned long type;};
template <> struct right_int_getter<8> {typedef unsigned long long type;};

struct ptr2int
{
    typedef right_int_getter<sizeof(void*)>::type type;
	static type& cast(void*& p) {return *(type*)(&p);}
	static type& cast(void*const& p) {return *(type*)(&p);}
	static void*& cast(type& i){return *(void**)(&i);};
	static void*& cast(type const& i){return *(void**)(&i);};
};

template <class CharType>
inline void serialize_load(const iarchive_pchar<CharType>& ar, void*& t)
{
	ar.to_integer(ptr2int::cast(t));
}

template <class CharType>
inline void serialize_save(oarchive_pchar<CharType>& ar, void* const& t)
{
	ar.hex_from_integer(ptr2int::cast(t));
}


#undef STRING_TRANSLATE_INTEGER_TYPE
#undef STRING_TRANSLATE_FLOAT_TYPE


NS_SERIALIZATION_LITE_END


#endif//__ARCHIVES_PCHAR_H_C1F2EE88_5119_4A06_A22B_53B4BD43E7F3__
