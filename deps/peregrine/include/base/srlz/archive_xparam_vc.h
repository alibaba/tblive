#ifndef __ARCHIVE_XPARAM_VC_H_AD81B9F0_043C_46de_861A_A5C30F60D657__
#define __ARCHIVE_XPARAM_VC_H_AD81B9F0_043C_46de_861A_A5C30F60D657__
#include "./archive_xparam_basic.h"
#include <cstringt.h>

NS_SERIALIZATION_LITE_BEGIN

//////////////////////////////////////////////////////////////////////////
// CString序列化
template <typename CharType, typename _Traits>
inline void serialize_load(iarchive_xparam const& ar, CStringT<CharType, _Traits>& t)
{
	if (ar.exists())
	{
		size_t vlen = 0;
		xparam::XInResP inres;
		CharType const* v = {0};
		if (ar.value(v, &vlen, inres))
			t.SetString(v, static_cast<int>(vlen));
	}
}

template <typename CharType, typename _Traits>
inline void serialize_save(oarchive_xparam& ar, CStringT<CharType, _Traits> const& t)
{
	ar.value(t.GetString(), t.GetLength());
}

//////////////////////////////////////////////////////////////////////////
// _bstr_t序列化
inline void serialize_load(iarchive_xparam const& ar, _bstr_t& t)
{
	if (ar.exists())
	{
		size_t vlen = 0;
		xparam::XInResP inres;
		wchar_t const* v = {0};
		if (ar.value(v, &vlen, inres))
			t.Attach(::SysAllocStringLen(v, static_cast<UINT>(vlen)));
	}
}

inline void serialize_save(oarchive_xparam& ar, _bstr_t const& t)
{
	ar.value(t.operator const wchar_t*(), t.length());
}

NS_SERIALIZATION_LITE_END



#endif//__ARCHIVE_XPARAM_VC_H_AD81B9F0_043C_46de_861A_A5C30F60D657__
