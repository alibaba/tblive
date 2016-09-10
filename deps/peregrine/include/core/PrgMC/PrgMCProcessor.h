//------------------------------------------------------------------------------
//
//    版权所有(C) 阿里巴巴（中国）网络技术有限公司 保留所有权利
//
//    创建者:   徐明
//    创建日期: 2011-11-08
//    功能描述: 消息中心处理器基类
//
//
//------------------------------------------------------------------------------

#ifndef __PRGMCPROCESSOR_H_49B33FCC_A119_4397_899D_E95F430D4351_INCLUDED__
#define __PRGMCPROCESSOR_H_49B33FCC_A119_4397_899D_E95F430D4351_INCLUDED__

#include "PrgMCRegistration.h"
#include "PrgMCDispatcher.h"
#include <base/memory/ref_counted.h>

class CPrgMCProcessor
{
public:
    CPrgMCProcessor();
    virtual ~CPrgMCProcessor();

    virtual HRESULT Init(scoped_refptr<CPrgMCRegistration> spRegistration, int nFlag);
    virtual HRESULT UnInit();
    virtual HRESULT Process(const std::wstring& strMsg, prg::IXParam *pParam, prg::IXParam *pSender, int nFlag) = 0;
    virtual HRESULT GetDispatchMessageHandlers(const std::wstring& strMsg, DispatchHierarchyMessageHandlers &dispatchMessageHandlers);
    virtual HRESULT GetGlobalFilters(std::list<scoped_refptr<IPrgMsgHandler>> &globalFilters);

protected:
    scoped_refptr<CPrgMCRegistration> m_spRegistration;
};


#endif // #ifndef __PRGMCPROCESSOR_H_49B33FCC_A119_4397_899D_E95F430D4351_INCLUDED__
