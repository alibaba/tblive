//------------------------------------------------------------------------------
//
//    版权所有(C) 阿里巴巴（中国）网络技术有限公司 保留所有权利
//
//    创建者:   徐明
//    创建日期: 2015-01-29
//    功能描述: PrgCOM类的一些基础设施
//
//
//------------------------------------------------------------------------------

#ifndef __COMBase_H_A7FD4DC8_2445_49C6_AE6F_2B6CF296A471_INCLUDED__
#define __COMBase_H_A7FD4DC8_2445_49C6_AE6F_2B6CF296A471_INCLUDED__

#include "base/namespace.h"
#include "core/IPrgCOMRefCounted.h"
#include "core/PrgCOMRefCounted.h"
#include "core/IPrgCOMFactory.h"
#include "core/hresult_define.h"

BEGIN_NAMESPACE(prg)

struct PrgCOMInterfaceEntry
{
	std::string uuid;
	long offset;
};

template<class ThreadModel>
class CPrgCOMRootObject:public CPrgCOMRefCounted<ThreadModel>
{
public:
	virtual ~CPrgCOMRootObject() 
	{
	}

	ULONG InternalAddRef()
	{
		return CPrgCOMRefCounted<ThreadModel>::InternalAddRef();
	}

	ULONG InternalRelease()
	{
		return CPrgCOMRefCounted<ThreadModel>::InternalRelease();
	}

	HRESULT InternalQueryInterface(intptr_t pThis, PrgCOMInterfaceEntry *pEntry, const std::string &iid, void** ppvObject)
	{ 
		while (pEntry != NULL && (*pEntry).uuid.size() != 0) 
		{ 
			if (iid == i_uuidof(IPrgCOMRefCounted) || (*pEntry).uuid == iid) 
			{ 
				IPrgCOMRefCounted *pPrgCOMRefCounted = (IPrgCOMRefCounted*)(pThis + (*pEntry).offset); 
				pPrgCOMRefCounted->AddRef(); 
				*ppvObject = (void*)(pThis + (*pEntry).offset); 
				return S_OK; 
			} 
			pEntry++; 
		} 
		return E_PRGCORE_NOINTERFACE; 
	}
	
	virtual HRESULT FinalConstruct()
	{
		return S_OK;
	}
};

#define _OFFSET_PACKING 8
#define _offsetofclass(base, derived) ((long)(static_cast<base*>((derived*)_OFFSET_PACKING))-_OFFSET_PACKING)

#define BEGIN_PRGCOM_MAP(class) \
	public: \
	typedef class PrgCOMClass; \
	static prg::PrgCOMInterfaceEntry *GetPrgCOMInterfaceEntry() \
	{ \
		static prg::PrgCOMInterfaceEntry g_prgCOMInterfaceEntry[] = {

#define PRGCOM_INTERFACE_ENTRY(interface) \
			{i_uuidof(interface), _offsetofclass(interface, PrgCOMClass)},

#define PRGCOM_INTERFACE_ENTRY_IID(iid, interface) \
            {iid, _offsetofclass(interface, PrgCOMClass)},

#define END_PRGCOM_MAP() \
			{"", 0} \
		};\
		return g_prgCOMInterfaceEntry; \
	} \
	HRESULT _InternalQueryInterface(const std::string &iid, void** ppvObject) \
	{ \
		return InternalQueryInterface((intptr_t)this, PrgCOMClass::GetPrgCOMInterfaceEntry(), iid, ppvObject); \
	}\
	virtual ULONG REFCALLTYPE AddRef() = 0; \
	virtual ULONG REFCALLTYPE Release() = 0; \
	virtual HRESULT QueryInterface(const std::string &iid, void** ppvObject) = 0;

#define DECLARE_SINGLETON_REFCOUNT()\
	virtual ULONG REFCALLTYPE AddRef(){return 1314;}\
	virtual ULONG REFCALLTYPE Release(){return 1314;}


template<class T>
class CPrgCOMObject:public T
{
public:
	ULONG REFCALLTYPE AddRef()
	{
        return this->InternalAddRef();
	}

	ULONG REFCALLTYPE Release()
	{
		ULONG res = this->InternalRelease();
        if (res == 0)
		{
			delete this;
			return 0;
		}
		return res;
	}

	//不改变引用计数
	static HRESULT CreateInstance(CPrgCOMObject<T> **ppObject)
	{
		CPrgCOMObject<T> *p = new CPrgCOMObject<T>();
		if (p)
		{
			(*ppObject) = p;
			return S_OK;
		}
		return E_FAIL;
	}

	virtual HRESULT QueryInterface(const std::string &iid, void** ppvObject)
	{
        return this->_InternalQueryInterface(iid, ppvObject);
	}
};

template<class T>
class CPrgCOMObjectCreator
{
public:
	static HRESULT CreateInstance(const std::string &iid, void** ppvObject)
	{
		CPrgCOMObject<T> *pObject = NULL;
		HRESULT hr = CPrgCOMObject<T>::CreateInstance((CPrgCOMObject<T>**)&pObject);
		if (SUCCEEDED(hr) && pObject)
		{
			hr = pObject->QueryInterface(iid, ppvObject);
            if ( hr == S_OK ) {
                pObject->FinalConstruct();
            }
		}
		return hr;
	}
};

extern "C" IPrgCOMFactory* GetPrgCOMFactory();

template<class T>
class CPrgCOMObjectRuntime:public CPrgCOMObjectRuntimeBase
{
public:
	CPrgCOMObjectRuntime()
	{
		IPrgCOMFactory *pFactory = GetPrgCOMFactory();
		if (pFactory)
		{
			pFactory->AddPrgCOMRuntime(GetClsId(), this);
		}
	}

	HRESULT CreateInstance(const std::string &iid, void** ppvObject)
	{
		return CPrgCOMObjectCreator<T>::CreateInstance(iid, ppvObject);
	}

	std::string GetClsId()
	{
		return T::GetClsId();
	}

	std::string GetClsName()
	{
		return T::GetClsName();
	}

	std::string GetClsDesc()
	{
		return T::GetClsDesc();
	}

	std::vector<std::string> GetClsDependency()
	{
		return T::GetClsDependency();
	}
};

// gcc va_arg does not support non-pod types
//inline std::vector<std::string> GetDependsCLSID(int count, ...)
//{
//	std::vector<std::string> dependency;
//	va_list arg_ptr; 
//	va_start(arg_ptr, count); 
//	for(int i = 0; i < count; i++)
//	{
//		std::string clsid = va_arg(arg_ptr, std::string);
//		dependency.push_back(clsid);
//	}
//	va_end(arg_ptr);
//	return dependency;
//}

inline std::vector<std::string> GetDependsCLSID(std::string depend1 = "", std::string depend2 = "", std::string depend3 = "", std::string depend4 = "", std::string depend5 = "", std::string depend6 = "", std::string depend7 = "", std::string depend8 = "", std::string depend9 = "", std::string depend10 = "")
{
    std::vector<std::string> dependency;
    if (depend1.length() > 0) dependency.push_back(depend1);
    if (depend2.length() > 0) dependency.push_back(depend2);
    if (depend3.length() > 0) dependency.push_back(depend3);
    if (depend4.length() > 0) dependency.push_back(depend4);
    if (depend5.length() > 0) dependency.push_back(depend5);
    if (depend6.length() > 0) dependency.push_back(depend6);
    if (depend7.length() > 0) dependency.push_back(depend7);
    if (depend8.length() > 0) dependency.push_back(depend8);
    if (depend9.length() > 0) dependency.push_back(depend9);
    if (depend10.length() > 0) dependency.push_back(depend10);
    return dependency;
}


#define DECLARE_PRGCOM_RUNTIME(T, clsid, name, desc, dependency) \
	public: \
	static prg::CPrgCOMObjectRuntime<T> g_prgRuntime; \
	static std::string GetClsId() \
	{ \
		return clsid; \
	} \
	static std::string GetClsName() \
	{ \
		return name; \
	} \
	static std::string GetClsDesc() \
	{ \
		return desc; \
	} \
	static std::vector<std::string> GetClsDependency() \
	{ \
		return dependency; \
	}

#define IMPLEMENT_PRGCOM_RUNTIME(T) \
	prg::CPrgCOMObjectRuntime<T> T::g_prgRuntime;

END_NAMESPACE()
#endif  // __COMBase_H_A7FD4DC8_2445_49C6_AE6F_2B6CF296A471_INCLUDED__
