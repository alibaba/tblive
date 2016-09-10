//------------------------------------------------------------------------------
//
//    版权所有(C) 阿里巴巴（中国）网络技术有限公司 保留所有权利
//
//    创建者:   徐明
//    创建日期: 2011-11-07
//    功能描述: 消息中心的注册器
//
//
//------------------------------------------------------------------------------
#ifndef __PRGMCREGISTRATION_H_4DC15C70_E503_42CD_BADE_59553B82D922_INCLUDED__
#define __PRGMCREGISTRATION_H_4DC15C70_E503_42CD_BADE_59553B82D922_INCLUDED__


#include <list>
#if defined(OS_WIN)
#include <hash_map>
#else
#include <map>
#endif
#include <core/IPrgMC.h>
#include <core/IPrgMsgHandler.h>
#include <base/memory/ref_counted.h>
#include <base/memory/ref_ptrcounted.h>
#include "base/synchronization/lock.h"

struct HierarchyMessageHandlersWeakRef
{
    HierarchyMessageHandlersWeakRef();
    ~HierarchyMessageHandlersWeakRef();
    std::list<scoped_refptr<prg::IWeakRef>> topFilterMsgHandler;
    std::list<scoped_refptr<prg::IWeakRef>> filterMsgHandler;
    std::list<scoped_refptr<prg::IWeakRef>> topReceiverMsgHandler;
    std::list<scoped_refptr<prg::IWeakRef>> receiverMsgHandler;
};

struct HierarchyMessageHandlers
{
    HierarchyMessageHandlers();
    ~HierarchyMessageHandlers();
    std::list<scoped_refptr<IPrgMsgHandler>> topFilterMsgHandler;
    std::list<scoped_refptr<IPrgMsgHandler>> filterMsgHandler;
    std::list<scoped_refptr<IPrgMsgHandler>> topReceiverMsgHandler;
    std::list<scoped_refptr<IPrgMsgHandler>> receiverMsgHandler;
};

class CPrgMCRegistration:public base::RefCounted<CPrgMCRegistration>
{
public:
    CPrgMCRegistration();

    HRESULT Init();
    HRESULT UnInit();

    HRESULT RegisterMessage(const std::wstring& strMsg, IPrgMsgHandler *pHandler, prg::REGISTERTYPE type);
    HRESULT UnRegisterMessage(const std::wstring& strMsg, IPrgMsgHandler *pHandler);

    HRESULT RegisterGlobalFilter(IPrgMsgHandler *pHandler);
    HRESULT UnRegisterGlobalFilter(IPrgMsgHandler *pHandler);
    
    HRESULT GetRegisterMessages(const std::wstring& strMsg, HierarchyMessageHandlers &handlers);
    HRESULT GetGlobalFilters(std::list<scoped_refptr<IPrgMsgHandler>> &globalFilters);

private:
    friend class base::RefCounted<CPrgMCRegistration>;
    virtual ~CPrgMCRegistration();
    
private:
    void GetHandlersStrongRef(std::list<scoped_refptr<prg::IWeakRef>> &handlersWeakRef, std::list<scoped_refptr<IPrgMsgHandler>> &handlers);

private:
    bool m_bInited;
#if defined(OS_WIN)
    stdext::hash_map<std::wstring, HierarchyMessageHandlersWeakRef> m_mapMsgHandlers;
#else
    std::map<std::wstring, HierarchyMessageHandlersWeakRef> m_mapMsgHandlers;
#endif
    base::Lock m_lock;
    std::list<scoped_refptr<prg::IWeakRef>> m_listGlobalFilters;
};

#endif // #ifndef __PRGMCREGISTRATION_H_4DC15C70_E503_42CD_BADE_59553B82D922_INCLUDED__
