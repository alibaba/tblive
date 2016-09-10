#ifndef __ARCHIVE_MSXML_VC_H_AD81B9F0_043C_46de_861A_A5C30F60D657__
#define __ARCHIVE_MSXML_VC_H_AD81B9F0_043C_46de_861A_A5C30F60D657__
#include "./archive_msxml_basic.h"
#include "./archive_pchar_vc.h"
#include <cstringt.h>

NS_SERIALIZATION_LITE_BEGIN

//////////////////////////////////////////////////////////////////////////
// CStringT–Ú¡–ªØ

template <typename CharType, typename _Traits>
inline void serialize_load(iarchive_msxml const& ar, CStringT<CharType, _Traits>& t)
{
	if (ar.exists())
	{
		_bstr_t bstr = ar.value();
		size_t len = sizeof(CharType) == sizeof(char) ? ZERO_TERM : bstr.length();
		iarchive_pchar<CharType>(bstr, len) >> t;
	}
}

template <typename CharType, typename _Traits>
inline void serialize_load(iarchive_msxml_attr const& ar, CStringT<CharType, _Traits>& t)
{
	if (ar.exists())
	{
		_bstr_t bstr = ar.value();
		size_t len = sizeof(CharType) == sizeof(char) ? ZERO_TERM : bstr.length();
		iarchive_pchar<CharType>(bstr, len) >> t;
	}
}

template <typename CharType, typename _Traits>
inline void serialize_save(oarchive_msxml& ar, CStringT<CharType, _Traits> const& t)
{
	ar.value(t.GetString());
}

template <typename CharType, typename _Traits>
inline void serialize_save(oarchive_msxml_attr& ar, CStringT<CharType, _Traits> const& t)
{
	ar.value(t.GetString());
}

NS_SERIALIZATION_LITE_END

#endif//__ARCHIVE_MSXML_VC_H_AD81B9F0_043C_46de_861A_A5C30F60D657__
