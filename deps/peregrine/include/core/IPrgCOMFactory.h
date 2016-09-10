//------------------------------------------------------------------------------
//
//    版权所有(C) 阿里巴巴（中国）网络技术有限公司 保留所有权利
//
//    创建者:   徐明
//    创建日期: 2015-01-29
//    功能描述: PrgCOM类的类工厂，每一个dll一份，定义统一接口
//
//
//------------------------------------------------------------------------------

#ifndef __IPrgCOMFactory_H_A7FD4DC8_2445_49C6_AE6F_2B6CF296A471_INCLUDED__
#define __IPrgCOMFactory_H_A7FD4DC8_2445_49C6_AE6F_2B6CF296A471_INCLUDED__

#include "base/namespace.h"
#include "base/memory/iref_counted.h"
#include "base/memory/ref_counted.h"
#include "base/prg.h"
#include "string"
#include "vector"

BEGIN_NAMESPACE(prg)

class CPrgCOMObjectRuntimeBase
{
public:
	virtual HRESULT CreateInstance(const std::string &iid, void** ppvObject) = 0;
	virtual std::string GetClsId() = 0;
	virtual std::string GetClsName() = 0;
	virtual std::string GetClsDesc() = 0;
	virtual std::vector<std::string> GetClsDependency() = 0;
};

class IPrgCOMEnum:public base::IRefCounted
{
public:
	virtual bool Next() = 0;
	virtual CPrgCOMObjectRuntimeBase* Current() = 0;
};

class IPrgCOMFactory
{
public:
	virtual std::string GetName() = 0;
	virtual std::string GetDesc() = 0;
	virtual scoped_refptr<IPrgCOMEnum> GetPrgCOMEnum() = 0;
	virtual HRESULT CreatePrgCOM(const std::string &clsid, const std::string &iid, void** ppvObject) = 0;
	virtual void AddPrgCOMRuntime(const std::string &clsid, CPrgCOMObjectRuntimeBase *pCOMRuntime) = 0;
	//virtual void Lock() = 0;
	//virtual void Unlock() = 0;
	//virtual bool FactoryCanUnloadNow() = 0;
};

END_NAMESPACE()
#endif  // __IPrgCOMFactory_H_A7FD4DC8_2445_49C6_AE6F_2B6CF296A471_INCLUDED__
