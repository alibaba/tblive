//------------------------------------------------------------------------------
//
//    版权所有(C) 阿里巴巴（中国）网络技术有限公司 保留所有权利
//
//    创建者:   徐明
//    创建日期: 2015-02-02
//    功能描述: PrgCOM的运行时库，包含Cache列表
//
//
//------------------------------------------------------------------------------

#ifndef __PrgComRuntime_H_441BCECA_FF22_42EE_BFD5_18CC1B2A3D22_INCLUDED__
#define __PrgComRuntime_H_441BCECA_FF22_42EE_BFD5_18CC1B2A3D22_INCLUDED__

#include "core/IPrgCOM.h"
#include "core/IPrgCOMRefCounted.h"
#include "base/memory/ref_ptrcounted.h"
#include "map"
#include "PrgCOMCreator.h"
#include "base/synchronization/lock.h"
#include "base/memory/ref_counted.h"

class InstanceCollection:public base::RefCountedThreadSafe<InstanceCollection>
{
public:
    InstanceCollection();
    
    std::map<std::string, scoped_refptr<prg::IPrgCOMRefCounted>> instances;
    base::Lock lock;
    
private:
    friend class base::RefCountedThreadSafe<InstanceCollection>;
    ~InstanceCollection();
};

class CPrgCOMRuntime:public base::RefPtrCounted<prg::IPrgCOM, base::ThreadSafeRefPolicy>
{
public:
    CPrgCOMRuntime();
    virtual ~CPrgCOMRuntime();

    virtual HRESULT Init(const base::FilePath &path) OVERRIDE;
	virtual HRESULT InitCustomPrgCOMObjectConfig(const base::FilePath &userPath, const std::string &configid = "Id1") OVERRIDE;
    virtual HRESULT Uninit() OVERRIDE;
    virtual HRESULT CreateInstance(const std::string &clsid, const std::string &iid, void **ppObject, const std::string &configid = "") OVERRIDE;
    virtual HRESULT GetInstance(const std::string &clsid, const std::string &name, const std::string &iid, void **ppObject, const std::string &configid = "", const std::string &groupId = "") OVERRIDE;
    virtual HRESULT HasInstance(const std::string &clsid, const std::string &name, bool *bHas, const std::string &groupId = "") OVERRIDE;
    virtual HRESULT DropInstance(const std::string &clsid, const std::string &name, const std::string &groupId = "") OVERRIDE;
	virtual HRESULT DropInstanceGroup(const std::string &groupId = "") OVERRIDE;
	virtual HRESULT CanCreateInstance(const std::string &clsid, bool *bCan, const std::string &configid = "") OVERRIDE;
	virtual HRESULT GetPrgCOMObjectConfig(std::list<scoped_refptr<prg::CPrgCOMObjectIdentify>> &listPrgCOMObject, const std::string &configid = "") OVERRIDE;
	virtual HRESULT ResetCustomPrgCOMObjectConfig(const std::string &configid = "Id1") OVERRIDE;
	virtual HRESULT EnableCustomPrgCOMObjectConfig(const std::string &clsid, bool enable = true, const std::string &configid = "Id1") OVERRIDE;

private:
    scoped_refptr<InstanceCollection> GetInstances4Clsid(const std::string &groupId, const std::string &clsid);

private:
	// <groupId, <clsid, instances>>
	std::map<std::string, std::map<std::string, scoped_refptr<InstanceCollection>>> m_runtimeObjs;
    base::Lock m_lock;
    CPrgCOMCreator m_creator;
    bool m_bInited;
};

#endif // #ifndef __PrgComRuntime_H_441BCECA_FF22_42EE_BFD5_18CC1B2A3D22_INCLUDED__
