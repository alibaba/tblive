//------------------------------------------------------------------------------
//
//  版权所有(C) 浙江天猫技术有限公司 保留所有权利
//
//  创建者:   哈尔
//  创建日期: 2015/02/11
//  功能描述: 消息中心帮助类
//
//------------------------------------------------------------------------------

#ifndef __MSGCENTER_HELPER_H_15813270_89E5_4B2D_B99B_8B8B4CD4C38B_INCLUDED__
#define __MSGCENTER_HELPER_H_15813270_89E5_4B2D_B99B_8B8B4CD4C38B_INCLUDED__

#include <core/PrgCore.h>
#include <core/IPrgMC.h>
#include <core/IPrgMsgHandler.h>
#include <base/namespace.h>
#include <base/memory/iref_counted_wrap.h>

BEGIN_NAMESPACE(prg)

class msgcenter : public prg::IRefcountWrapBase<prg::IPrgMC, msgcenter>
{
public:
    msgcenter(const std::wstring& msgcentername = L"")
    {
        GetPrgMessageCenter(msgcentername, m_spInterface.getaddress());
    }

    HRESULT Init(int nAsyncMaxThread = 1)
    {
        return m_spInterface->Init(nAsyncMaxThread);
    }

    HRESULT UnInit()
    {
        return m_spInterface->UnInit();
    }

    HRESULT RegisterMessage(const std::wstring& strMsg, IPrgMsgHandler *pHandler, prg::REGISTERTYPE type)
    {
        return m_spInterface->RegisterMessage(strMsg, pHandler, type);
    }

    HRESULT UnRegisterMessage(const std::wstring& strMsg, IPrgMsgHandler *pHandler)
    {
        return m_spInterface->UnRegisterMessage(strMsg, pHandler);
    }

    HRESULT SendMessage(const std::wstring& strMsg, prg::IXParam *pParam = NULL, prg::IXParam *pSender = NULL)
    {
        return m_spInterface->SendMessage(strMsg, pParam, pSender);
    }

    HRESULT PostMessage(const std::wstring& strMsg, prg::IXParam *pParam = NULL, prg::IXParam *pSender = NULL, prg::ASYNCPRIORITYTYPE type = ASYNCPRIORITYTYPE_NORMAL)
    {
        return m_spInterface->PostMessage(strMsg, pParam, pSender, type);
    }

    HRESULT RegisterGlobalFilter(IPrgMsgHandler *pHandler)
    {
        return m_spInterface->RegisterGlobalFilter(pHandler);
    }
        
    HRESULT UnRegisterGlobalFilter(IPrgMsgHandler *pHandler)
    {
        return m_spInterface->UnRegisterGlobalFilter(pHandler);
    }

    HRESULT ClearPostMessages()
    {
        return m_spInterface->ClearPostMessages();
    }
};

END_NAMESPACE()

#endif // __MSGCENTER_HELPER_H_15813270_89E5_4B2D_B99B_8B8B4CD4C38B_INCLUDED__