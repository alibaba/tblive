#ifndef __ARCHIVE_XPARAM_COM_H_AD81B9F0_043C_46de_861A_A5C30F60D657__
#define __ARCHIVE_XPARAM_COM_H_AD81B9F0_043C_46de_861A_A5C30F60D657__
#include "./archive_xparam_basic.h"
#include <atlcomcli.h>
#include <comip.h>

NS_SERIALIZATION_LITE_BEGIN

//////////////////////////////////////////////////////////////////////////
// IUnknown
inline static void serialize_pointer_load(IUnknown*, iarchive_xparam const& ar, IUnknown*& t)
{
	IUnknown* pUnk = NULL;
	if(ar.value(pUnk) && pUnk)
		t = pUnk;
}
template <typename T>
inline static void serialize_pointer_load(IUnknown*, iarchive_xparam const& ar, T*& t)
{
	IUnknown* pUnk = NULL;
	if(ar.value(pUnk) && pUnk)
	{
		T* pt = NULL;
		if (SUCCEEDED(pUnk->QueryInterface(__uuidof(T), (void**)&pt)))
			t = pt;
		pUnk->Release();
	}
}
template <typename T>
inline static void serialize_pointer_save(IUnknown*, oarchive_xparam& ar, T*const& t)
{
	ar.value(static_cast<IUnknown*>(t));
}

//////////////////////////////////////////////////////////////////////////
// ATL CComPtr
inline static void serialize_load(iarchive_xparam const& ar, ATL::CComPtr<IUnknown>& t)
{
	IUnknown* pUnk = NULL;
	if(ar.value(pUnk) && pUnk)
		t.Attach(pUnk);
}
template <typename T>
inline static void serialize_load(iarchive_xparam const& ar, ATL::CComPtr<T>& t)
{
	IUnknown* pUnk = NULL;
	if(ar.value(pUnk) && pUnk)
	{
		T* pt = NULL;
		if (SUCCEEDED(pUnk->QueryInterface(__uuidof(T), (void**)&pt)))
			t.Attach(pt);
		pUnk->Release();
	}
}
template <typename T>
inline static void serialize_save(oarchive_xparam& ar, ATL::CComPtr<T> const& t)
{
	ar.value(static_cast<IUnknown*>((T*)t));
}

//////////////////////////////////////////////////////////////////////////
// _com_ptr_t
template <const IID* _IID>
inline static void serialize_load(iarchive_xparam const& ar, _com_ptr_t< _com_IIID<IUnknown, _IID> >& t)
{
	IUnknown* pUnk = NULL;
	if(ar.value(pUnk) && pUnk)
		t.Attach(pUnk);
}

template <typename T>
inline static void serialize_load(iarchive_xparam const& ar, _com_ptr_t<T>& t)
{
	IUnknown* pUnk = NULL;
	if(ar.value(pUnk) && pUnk)
	{
		T::Interface* pt = NULL;
		if (SUCCEEDED(pUnk->QueryInterface(T::GetIID(), (void**)&pt)))
			t.Attach(pt);
		pUnk->Release();
	}
}
template <typename T>
inline static void serialize_save(oarchive_xparam& ar, _com_ptr_t<T> const& t)
{
	ar.value(static_cast<IUnknown*>((T::Interface*)t));
}

NS_SERIALIZATION_LITE_END



#endif//__ARCHIVE_XPARAM_COM_H_AD81B9F0_043C_46de_861A_A5C30F60D657__
