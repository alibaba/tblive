//------------------------------------------------------------------------------
//
//    版权所有(C) 阿里巴巴（中国）网络技术有限公司 保留所有权利
//
//    创建者:   徐明
//    创建日期: 2011-11-08
//    功能描述: 消息回调器
//
//
//------------------------------------------------------------------------------

#ifndef __PRGMCDISPATCHER_H_0642B9BE_396C_42EA_8230_9FB8C344A688_INCLUDED__
#define __PRGMCDISPATCHER_H_0642B9BE_396C_42EA_8230_9FB8C344A688_INCLUDED__


#include <list>
#include <core/IPrgMsgHandler.h>
#include <base/xparam/IXParam.h>
#include <base/memory/ref_counted.h>

struct DispatchMessageHandler
{
    DispatchMessageHandler(scoped_refptr<IPrgMsgHandler> spHandler, const std::wstring &strMsg);
    ~DispatchMessageHandler();
    scoped_refptr<IPrgMsgHandler> spMsgHandler;
    std::wstring strRegisterMsg;
};

struct DispatchHierarchyMessageHandlers
{
    DispatchHierarchyMessageHandlers();
    ~DispatchHierarchyMessageHandlers();
    
    std::list<DispatchMessageHandler> topFilterMsgHandler;
    std::list<DispatchMessageHandler> filterMsgHandler;
    std::list<DispatchMessageHandler> topReceiverMsgHandler;
    std::list<DispatchMessageHandler> receiverMsgHandler;
};

class CPrgMCDispatcher
{
public:
    static HRESULT Invoke(std::list<scoped_refptr<IPrgMsgHandler>> globalFilters,
                          const std::wstring& strMsg, const DispatchHierarchyMessageHandlers &handlers, prg::IXParam *pParam, prg::IXParam *pSender);

private:
    CPrgMCDispatcher();
};



#endif // #ifndef __PRGMCDISPATCHER_H_0642B9BE_396C_42EA_8230_9FB8C344A688_INCLUDED__
