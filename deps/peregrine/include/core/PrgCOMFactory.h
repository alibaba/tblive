//------------------------------------------------------------------------------
//
//    版权所有(C) 阿里巴巴（中国）网络技术有限公司 保留所有权利
//
//    创建者:   徐明
//    创建日期: 2015-01-29
//    功能描述: PrgCOM类的类工厂，每一个dll一份
//
//
//------------------------------------------------------------------------------

#ifndef __PrgCOMFactory_H_A7FD4DC8_2445_49C6_AE6F_2B6CF296A471_INCLUDED__
#define __PrgCOMFactory_H_A7FD4DC8_2445_49C6_AE6F_2B6CF296A471_INCLUDED__

#include "base/namespace.h"
#include "core/IPrgCOMFactory.h"
#include "base/memory/ref_ptrcounted.h"
#include "base/memory/ref_counted.h"
#include "map"
#include "core/hresult_define.h"

BEGIN_NAMESPACE(prg)

class CPrgCOMEnum:public base::RefPtrCounted<IPrgCOMEnum, base::ThreadSafeRefPolicy>
{
public:
	CPrgCOMEnum(std::map<std::string, CPrgCOMObjectRuntimeBase*> runtimes):mapClsId2PrgCOMRuntime(runtimes), started(false)
	{
		currentPrgCOMRuntime = mapClsId2PrgCOMRuntime.end();
	}

	bool Next()
	{
		if (!started)
		{
			started = true;
			currentPrgCOMRuntime = mapClsId2PrgCOMRuntime.begin();
			if (currentPrgCOMRuntime != mapClsId2PrgCOMRuntime.end())
			{
				return true;
			}
			return false;
		}
		else
		{
			if (currentPrgCOMRuntime != mapClsId2PrgCOMRuntime.end())
			{
				currentPrgCOMRuntime++;
				if (currentPrgCOMRuntime != mapClsId2PrgCOMRuntime.end())
				{
					return true;
				}
			}
			return false;
		}
	}

	CPrgCOMObjectRuntimeBase* Current()
	{
		if (currentPrgCOMRuntime != mapClsId2PrgCOMRuntime.end())
		{
			return currentPrgCOMRuntime->second;
		}

		return NULL;
	}

private:
	bool started;
	std::map<std::string, CPrgCOMObjectRuntimeBase*> mapClsId2PrgCOMRuntime;
	std::map<std::string, CPrgCOMObjectRuntimeBase*>::iterator currentPrgCOMRuntime;
};

class CPrgCOMFactory:public IPrgCOMFactory
{
public:
	CPrgCOMFactory(const std::string &_name, const std::string _desc):name(_name), desc(_desc)
	{
	}

	std::string GetName()
	{
		return name;
	}

	std::string GetDesc()
	{
		return desc;
	}

	void AddPrgCOMRuntime(const std::string &clsid, CPrgCOMObjectRuntimeBase *pCOMRuntime)
	{
		mapClsId2PrgCOMRuntime[clsid] = pCOMRuntime;
	}

	scoped_refptr<IPrgCOMEnum> GetPrgCOMEnum()
	{
		scoped_refptr<CPrgCOMEnum> spEnum = new CPrgCOMEnum(mapClsId2PrgCOMRuntime);
		return spEnum;
	}

	HRESULT CreatePrgCOM(const std::string &clsid, const std::string &iid, void** ppvObject)
	{
		auto it = mapClsId2PrgCOMRuntime.find(clsid);
		if (it != mapClsId2PrgCOMRuntime.end())
		{
			return it->second->CreateInstance(iid, ppvObject);
		}
		return E_PRGCORE_HASNOCLSID;
	}

private:
	std::string name;
	std::string desc;
	std::map<std::string, CPrgCOMObjectRuntimeBase*> mapClsId2PrgCOMRuntime;
};

#define DECLARE_PRGCOM_FACTORY(name, desc) \
	extern "C" Prg_API_Export prg::IPrgCOMFactory* GetPrgCOMFactory() \
	{ \
        CR_DEFINE_STATIC_LOCAL(prg::CPrgCOMFactory, g_prgCOMFactory, (name, desc)); \
        return (prg::IPrgCOMFactory*)(&g_prgCOMFactory); \
	}

END_NAMESPACE()
#endif  // __PrgCOMFactory_H_A7FD4DC8_2445_49C6_AE6F_2B6CF296A471_INCLUDED__
