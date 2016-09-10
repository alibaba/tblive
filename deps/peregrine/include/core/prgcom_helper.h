//------------------------------------------------------------------------------
//
//    版权所有(C) 阿里巴巴（中国）网络技术有限公司 保留所有权利
//
//    创建者:   徐明
//    创建日期: 2015-02-10
//    功能描述: 创建com的c方法封装。
//
//
//------------------------------------------------------------------------------

#ifndef __PrgCOM_HELPER_H_75895926_20F7_40B4_A1F4_718D2F4AFFA1_INCLUDED__
#define __PrgCOM_HELPER_H_75895926_20F7_40B4_A1F4_718D2F4AFFA1_INCLUDED__

#include "string"
#include "base/namespace.h"
#include "base/memory/iref_counted.h"
#include "base/memory/ref_counted.h"
#include "core/PrgQIRefPtr.h"
#include "list"
#include "core/IPrgCOM.h"
#include "core/PrgCore.h"

BEGIN_NAMESPACE(prg)

inline HRESULT PrgCOMInit(const base::FilePath &dir)
{
	scoped_refptr<IPrgCOM> spPrgCOM;
	GetPrgCOM((prg::IPrgCOM**)spPrgCOM.getaddress());
	if (spPrgCOM)
	{
		return spPrgCOM->Init(dir);
	}
	return E_FAIL;
}

inline HRESULT PrgCOMInitCustomObjectConfig(const std::string &configid, const base::FilePath &dir)
{
	scoped_refptr<IPrgCOM> spPrgCOM;
	GetPrgCOM((prg::IPrgCOM**)spPrgCOM.getaddress());
	if (spPrgCOM)
	{
		return spPrgCOM->InitCustomPrgCOMObjectConfig(dir, configid);
	}
	return E_FAIL;
}

inline HRESULT PrgCOMUninit()
{
	scoped_refptr<IPrgCOM> spPrgCOM;
	GetPrgCOM((prg::IPrgCOM**)spPrgCOM.getaddress());
	if (spPrgCOM)
	{
		return spPrgCOM->Uninit();
	}
	return E_FAIL;
}

inline HRESULT PrgCOMCreateInstance(const std::string &clsid, const std::string &iid, void **ppObject, const std::string &configid = "")
{
	scoped_refptr<IPrgCOM> spPrgCOM;
	GetPrgCOM((prg::IPrgCOM**)spPrgCOM.getaddress());
	if (spPrgCOM)
	{
		return spPrgCOM->CreateInstance(clsid, iid, ppObject, configid);
	}
	return E_FAIL;
}

template<typename T>
inline HRESULT PrgCOMCreateInstance(const std::string &clsid, prg::CPrgQIRefPtr<T> &spInterface, const std::string &configid = "")
{
	spInterface = NULL;
	scoped_refptr<IPrgCOM> spPrgCOM;
	GetPrgCOM((prg::IPrgCOM**)spPrgCOM.getaddress());
	if (spPrgCOM)
	{
		return spPrgCOM->CreateInstance(clsid, i_uuidof(T), (void**)spInterface.getaddress(), configid);
	}
	return E_FAIL;
}

inline HRESULT PrgCOMGetInstance(const std::string &clsid, const std::string &name, const std::string &iid, void **ppObject, const std::string &configid = "", const std::string &groupId = "")
{
	scoped_refptr<IPrgCOM> spPrgCOM;
	GetPrgCOM((prg::IPrgCOM**)spPrgCOM.getaddress());
	if (spPrgCOM)
	{
		return spPrgCOM->GetInstance(clsid, name, iid, ppObject, configid, groupId);
	}
	return E_FAIL;
}

template<typename T>
inline HRESULT PrgCOMGetInstance(const std::string &clsid, const std::string &name, scoped_refptr<T> &spInterface, const std::string &configid = "", const std::string &groupId = "")
{
	spInterface = NULL;
	prg::CPrgQIRefPtr<T> spQIInterface;
	HRESULT hr = PrgCOMGetInstance( clsid, name, spQIInterface, configid, groupId );
	if ( SUCCEEDED(hr) ) {
		spInterface = make_scoped_refptr( spQIInterface.get() );
	}
 	return hr;
}

template<typename T>
inline HRESULT PrgCOMGetInstance(const std::string &clsid, const std::string &name, prg::CPrgQIRefPtr<T> &spInterface, const std::string &configid = "", const std::string &groupId = "")
{
	spInterface = NULL;
	scoped_refptr<IPrgCOM> spPrgCOM;
	GetPrgCOM((prg::IPrgCOM**)spPrgCOM.getaddress());
	if (spPrgCOM)
	{
		return spPrgCOM->GetInstance(clsid, name, i_uuidof(T), (void**)spInterface.getaddress(), configid, groupId);
	}
	return E_FAIL;
}

inline HRESULT PrgCOMHasInstance(const std::string &clsid, const std::string &name, bool *pHave, const std::string &groupId = "")
{
	scoped_refptr<IPrgCOM> spPrgCOM;
	GetPrgCOM((prg::IPrgCOM**)spPrgCOM.getaddress());
	if (spPrgCOM)
	{
		return spPrgCOM->HasInstance(clsid, name, pHave, groupId);
	}
	return E_FAIL;
}

inline HRESULT PrgCOMDropInstance(const std::string &clsid, const std::string &name, const std::string &groupId = "")
{
	scoped_refptr<IPrgCOM> spPrgCOM;
	GetPrgCOM((prg::IPrgCOM**)spPrgCOM.getaddress());
	if (spPrgCOM)
	{
		return spPrgCOM->DropInstance(clsid, name, groupId);
	}
	return E_FAIL;
}

inline HRESULT PrgCOMDropInstanceGroup(const std::string &groupId = "")
{
	scoped_refptr<IPrgCOM> spPrgCOM;
	GetPrgCOM((prg::IPrgCOM**)spPrgCOM.getaddress());
	if (spPrgCOM)
	{
		return spPrgCOM->DropInstanceGroup(groupId);
	}
	return E_FAIL;
}

inline HRESULT PrgCOMGetObjectConfig(std::list<scoped_refptr<CPrgCOMObjectIdentify>> &listPrgCOMObject, const std::string &configid = "")
{
	scoped_refptr<IPrgCOM> spPrgCOM;
	GetPrgCOM((prg::IPrgCOM**)spPrgCOM.getaddress());
	if (spPrgCOM)
	{
		return spPrgCOM->GetPrgCOMObjectConfig(listPrgCOMObject, configid);
	}
	return E_FAIL;
}

inline HRESULT PrgCOMResetCustomObjectConfig(const std::string &configid)
{
	scoped_refptr<IPrgCOM> spPrgCOM;
	GetPrgCOM((prg::IPrgCOM**)spPrgCOM.getaddress());
	if (spPrgCOM)
	{
		return spPrgCOM->ResetCustomPrgCOMObjectConfig(configid);
	}
	return E_FAIL;
}

inline HRESULT PrgCOMEnableCustomObjectConfig(const std::string &configid, const std::string &clsid, bool enable = true)
{
	scoped_refptr<IPrgCOM> spPrgCOM;
	GetPrgCOM((prg::IPrgCOM**)spPrgCOM.getaddress());
	if (spPrgCOM)
	{
		return spPrgCOM->EnableCustomPrgCOMObjectConfig(clsid, enable, configid);
	}
	return E_FAIL;
}

END_NAMESPACE()
#endif // #ifndef __PrgCOM_HELPER_H_75895926_20F7_40B4_A1F4_718D2F4AFFA1_INCLUDED__
