#ifndef __ARCHIVES_PCHAR_VC_H_C1F2EE88_5119_4A06_A22B_53B4BD43E7F3__
#define __ARCHIVES_PCHAR_VC_H_C1F2EE88_5119_4A06_A22B_53B4BD43E7F3__
#include "./archive_pchar.h"
#include <cstringt.h>

NS_SERIALIZATION_LITE_BEGIN

//////////////////////////////////////////////////////////////////////////
// for CStringT
template <class CharType, typename _Traits>
inline void serialize_load(iarchive_pchar<CharType> const& ar, CStringT<CharType, _Traits>& t)
{
	if (ar.len)
		t.SetString(ar.pchar, static_cast<int>(ar.len));
	else
		t.Empty();
}

template <class CharType, typename _Traits>
inline void serialize_save(oarchive_pchar<CharType>& ar, CStringT<CharType, _Traits> const& t)
{
	ar.realloc(t.GetLength() + 1);
	memcpy(ar.pchar, t.GetString(), sizeof(CharType) * t.GetLength());
	ar.pchar[--ar.len] = 0;
}

NS_SERIALIZATION_LITE_END



#endif//__ARCHIVES_PCHAR_VC_H_C1F2EE88_5119_4A06_A22B_53B4BD43E7F3__