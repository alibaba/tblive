//------------------------------------------------------------------------------
//
//    版权所有(C) 阿里巴巴（中国）网络技术有限公司 保留所有权利
//
//    创建者:   徐明
//    创建日期: 2011-11-08
//    功能描述: 异步消息处理器
//
//
//------------------------------------------------------------------------------

#ifndef __PRGMCASYNCPROCESSOR_H_BE844513_5EF7_4DF8_9634_17AF0677D223_INCLUDED__
#define __PRGMCASYNCPROCESSOR_H_BE844513_5EF7_4DF8_9634_17AF0677D223_INCLUDED__

#include "PrgMCProcessor.h"
#include "base/synchronization/lock.h"
#include <base/threading/sequenced_worker_pool.h>
#include <base/memory/ref_counted.h>
#include <base/xparam/IXParam.h>
#include <list>

struct AsyncMessage
{
    AsyncMessage();
    AsyncMessage(const std::wstring &msg, prg::IXParam *pParam, prg::IXParam *pSender);
    AsyncMessage(const AsyncMessage& other);
    ~AsyncMessage();
    
    std::wstring strMsg;
    scoped_refptr<prg::IXParam> spParam;
    scoped_refptr<prg::IXParam> spSender;
};

class PriorityMsgsQueue
{
public:
    PriorityMsgsQueue();
    ~PriorityMsgsQueue();
    
    void PushMsg(const AsyncMessage &strMsg, int nPriority)
    {
        base::AutoLock lk(m_lock);
        prg::ASYNCPRIORITYTYPE priority = (prg::ASYNCPRIORITYTYPE)nPriority;
        switch (priority)
        {
            case prg::ASYNCPRIORITYTYPE_TOP:
                topMsgs.push_back(strMsg);
                break;
            case prg::ASYNCPRIORITYTYPE_NORMAL:
                normalMsgs.push_back(strMsg);
                break;
            case prg::ASYNCPRIORITYTYPE_IDLE:
                idleMsgs.push_back(strMsg);
                break;
        }
    }
    
    AsyncMessage PopMsg()
    {
        base::AutoLock lk(m_lock);
        if (topMsgs.size() > 0)
        {
            AsyncMessage strMsg = *topMsgs.begin();
            topMsgs.pop_front();
            return strMsg;
        }
        if (normalMsgs.size() > 0)
        {
            AsyncMessage strMsg = *normalMsgs.begin();
            normalMsgs.pop_front();
            return strMsg;
        }
        if (idleMsgs.size() > 0)
        {
            AsyncMessage strMsg = *idleMsgs.begin();
            idleMsgs.pop_front();
            return strMsg;
        }
        return AsyncMessage();
    }
    
    void clear()
    {
        base::AutoLock lk(m_lock);
        topMsgs.clear();
        normalMsgs.clear();
        idleMsgs.clear();
    }
private:
    std::list<AsyncMessage> topMsgs;
    std::list<AsyncMessage> normalMsgs;
    std::list<AsyncMessage> idleMsgs;
    base::Lock m_lock;
};

class CPrgMCAsyncProcessor:public CPrgMCProcessor, public base::RefCountedThreadSafe<CPrgMCAsyncProcessor>
{
public:
    CPrgMCAsyncProcessor();

    virtual HRESULT Init(scoped_refptr<CPrgMCRegistration> spRegistration, int nFlag) OVERRIDE;
    virtual HRESULT UnInit() OVERRIDE;
    virtual HRESULT Process(const std::wstring& strMsg, prg::IXParam *pParam, prg::IXParam *pSender, int nFlag) OVERRIDE;
    HRESULT ClearAsyncMessages();

private:
    friend class base::RefCountedThreadSafe<CPrgMCAsyncProcessor>;
    virtual ~CPrgMCAsyncProcessor();
    
public:
    void AsyncProcessTask();

private:
    bool m_bInited;
    PriorityMsgsQueue m_MsgQueue;
  base::Lock m_lock;;
    scoped_refptr<base::SequencedWorkerPool> m_spWorkPool;
};


#endif // #ifndef __PRGMCASYNCPROCESSOR_H_BE844513_5EF7_4DF8_9634_17AF0677D223_INCLUDED__
