//------------------------------------------------------------------------------
//
//    版权所有(C) 阿里巴巴（中国）网络技术有限公司 保留所有权利
//
//    创建者:   徐明
//    创建日期: 2012-04-06
//    功能描述: 消息中心注册器，方便一个c++类中注册多条消息
//
//------------------------------------------------------------------------------

#ifndef __MSGCENTER_REGISTRATION_H_1610FB0C_19A4_4C37_B7C6_82E61360C200_INCLUDED__
#define __MSGCENTER_REGISTRATION_H_1610FB0C_19A4_4C37_B7C6_82E61360C200_INCLUDED__

#include <base/namespace.h>
#include <base/xparam/IXParam.h>
#include <core/prgmc_helper.h>

#include <map>
#include "base/callback.h"
#include "base/bind.h"

#include "base/synchronization/lock.h"
#include "base/synchronization/condition_variable.h"
#include <core/SupportWeakRef.h>

BEGIN_NAMESPACE(prg)

class CMsgCenterRegHandlerBase;

typedef void (CMsgCenterRegHandlerBase:: *MSGCENTER_PROC0)();
typedef void (CMsgCenterRegHandlerBase:: *MSGCENTER_PROC1)(const std::wstring&);
typedef void (CMsgCenterRegHandlerBase:: *MSGCENTER_PROC2)(const std::wstring&, prg::IXParam *);
typedef void (CMsgCenterRegHandlerBase:: *MSGCENTER_PROC3)(const std::wstring&, prg::IXParam *, prg::IXParam *);
typedef void (CMsgCenterRegHandlerBase:: *MSGCENTER_PROC4)(const std::wstring&, prg::IXParam *, prg::IXParam *, bool &);

typedef base::Callback<void()>  MSGCENTER_FUN0;
typedef base::Callback<void(const std::wstring&)>  MSGCENTER_FUN1;
typedef base::Callback<void(const std::wstring&, prg::IXParam *)>  MSGCENTER_FUN2;
typedef base::Callback<void(const std::wstring&, prg::IXParam *, prg::IXParam *)>  MSGCENTER_FUN3;
typedef base::Callback<void(const std::wstring&, prg::IXParam *, prg::IXParam *, bool &)>  MSGCENTER_FUN4;

class MessageSinkEntry
{
public:
    union MSGCENTERPROC
    {
        void *       MsgProcInit;
        MSGCENTER_PROC0 MsgProc_0;
        MSGCENTER_PROC1 MsgProc_1;
        MSGCENTER_PROC2 MsgProc_2;
        MSGCENTER_PROC3 MsgProc_3;
        MSGCENTER_PROC4 MsgProc_4;
        MSGCENTER_FUN0  *pMsgFun_0;
        MSGCENTER_FUN1  *pMsgFun_1;
        MSGCENTER_FUN2  *pMsgFun_2;
        MSGCENTER_FUN3  *pMsgFun_3;
        MSGCENTER_FUN4  *pMsgFun_4;
    };

    enum MSGCENTERPROCTYPE
    {
        MsgCenterTypeError = -1,
        MsgCenterTypeBegin = 0,
        MsgCenterProcType0,
        MsgCenterProcType1,
        MsgCenterProcType2,
        MsgCenterProcType3,
        MsgCenterProcType4,
        MsgCenterFunType0,
        MsgCenterFunType1,
        MsgCenterFunType2,
        MsgCenterFunType3,
        MsgCenterFunType4,
        MsgCenterTypeAll,
    };

    MessageSinkEntry():nMsgBusProcType(MsgCenterTypeError)
    {
        functor.MsgProcInit = NULL;
    }
        
    MessageSinkEntry(const MessageSinkEntry &sink)
    {
        strMessageID = sink.strMessageID;
        strMsgCenterName = sink.strMsgCenterName;
        nMsgBusProcType = sink.nMsgBusProcType;
        functor = sink.functor;

        if (functor.MsgProcInit == NULL)
        {
            return;
        }

        switch(nMsgBusProcType)
        {
        case MsgCenterFunType0:
            {
                functor.pMsgFun_0 = new MSGCENTER_FUN0(*(sink.functor.pMsgFun_0));
            }
            break;
        case MsgCenterFunType1:
            {
                functor.pMsgFun_1 = new MSGCENTER_FUN1(*(sink.functor.pMsgFun_1));
            }
            break;
        case MsgCenterFunType2:
            {
                functor.pMsgFun_2 = new MSGCENTER_FUN2(*(sink.functor.pMsgFun_2));
            }
            break;
        case MsgCenterFunType3:
            {
                functor.pMsgFun_3 = new MSGCENTER_FUN3(*(sink.functor.pMsgFun_3));
            }
            break;
        case MsgCenterFunType4:
            {
                functor.pMsgFun_4 = new MSGCENTER_FUN4(*(sink.functor.pMsgFun_4));
            }
            break;
        }
    }
        
    MessageSinkEntry& operator = (const MessageSinkEntry &sink)
    {
        strMessageID = sink.strMessageID;
        strMsgCenterName = sink.strMsgCenterName;
        nMsgBusProcType = sink.nMsgBusProcType;
        functor = sink.functor;
            
        if (functor.MsgProcInit == NULL)
        {
            return *this;
        }

        switch(nMsgBusProcType)
        {
        case MsgCenterFunType0:
            {
                functor.pMsgFun_0 = new MSGCENTER_FUN0(*(sink.functor.pMsgFun_0));
            }
            break;
        case MsgCenterFunType1:
            {
                functor.pMsgFun_1 = new MSGCENTER_FUN1(*(sink.functor.pMsgFun_1));
            }
            break;
        case MsgCenterFunType2:
            {
                functor.pMsgFun_2 = new MSGCENTER_FUN2(*(sink.functor.pMsgFun_2));
            }
            break;
        case MsgCenterFunType3:
            {
                functor.pMsgFun_3 = new MSGCENTER_FUN3(*(sink.functor.pMsgFun_3));
            }
            break;
        case MsgCenterFunType4:
            {
                functor.pMsgFun_4 = new MSGCENTER_FUN4(*(sink.functor.pMsgFun_4));
            }
            break;
        }

        return *this;
    }

    ~MessageSinkEntry()
    {
        if (functor.MsgProcInit == NULL)
        {
            return;
        }

        switch(nMsgBusProcType)
        {
        case MsgCenterFunType0:
            {
                delete functor.pMsgFun_0;
            }
            break;
        case MsgCenterFunType1:
            {
                delete functor.pMsgFun_1;
            }
            break;
        case MsgCenterFunType2:
            {
                delete functor.pMsgFun_2;
            }
            break;
        case MsgCenterFunType3:
            {
                delete functor.pMsgFun_3;
            }
            break;
        case MsgCenterFunType4:
            {
                delete functor.pMsgFun_4;
            }
            break;
        }

        nMsgBusProcType = MsgCenterTypeError;
        functor.MsgProcInit = NULL;
    }
        
    void Invoke(
        CMsgCenterRegHandlerBase *pHandler, 
        const std::wstring& strMsgId, 
        prg::IXParam *pParameter, 
        prg::IXParam *pSender, 
        bool &bFiltered)
    {
        if(pHandler == NULL) return;
        if(nMsgBusProcType < MsgCenterTypeBegin || nMsgBusProcType >= MsgCenterTypeAll) return;
        if(functor.MsgProcInit == NULL) return;

        switch(nMsgBusProcType)
        {
        case MsgCenterProcType0:
            {
                (pHandler->*functor.MsgProc_0)();
            }
            break;
        case MsgCenterProcType1:
            {
                (pHandler->*functor.MsgProc_1)(strMsgId);
            }
            break;
        case MsgCenterProcType2:
            {
                (pHandler->*functor.MsgProc_2)(strMsgId, pParameter);
            }
            break;
        case MsgCenterProcType3:
            {
                (pHandler->*functor.MsgProc_3)(strMsgId, pParameter, pSender);
            }
            break;
        case MsgCenterProcType4:
            {
                (pHandler->*functor.MsgProc_4)(strMsgId, pParameter, pSender, bFiltered);
            }
            break;
        case MsgCenterFunType0:
            {
                (*functor.pMsgFun_0).Run();
            }
            break;
        case MsgCenterFunType1:
            {
                (*functor.pMsgFun_1).Run(strMsgId);
            }
            break;
        case MsgCenterFunType2:
            {
                (*functor.pMsgFun_2).Run(strMsgId, pParameter);
            }
            break;
        case MsgCenterFunType3:
            {
                (*functor.pMsgFun_3).Run(strMsgId, pParameter, pSender);
            }
            break;
        case MsgCenterFunType4:
            {
                (*functor.pMsgFun_4).Run(strMsgId, pParameter, pSender, bFiltered);
            }
            break;
        }
    }

    std::wstring    strMessageID;
    std::wstring    strMsgCenterName;
    int             nMsgBusProcType;
    MSGCENTERPROC   functor;
    unsigned long   dwFlags;
};

typedef std::map<std::wstring, MessageSinkEntry> MsgHandlerMap;

class CMsgCenterRegHandlerBase : public prg::CSupportWeakRef<IPrgMsgHandler>
{
public:
    CMsgCenterRegHandlerBase()
    {
    }

    virtual ~CMsgCenterRegHandlerBase()
    {
        RemoveAllRegisteredMessage();
    }

    virtual IPrgMsgHandler* GetMsgHandler()
    {
        IPrgMsgHandler * pHandler = this; 
        return pHandler;
    }

    enum HandlerEntryType
    {
        TopFilter,
        Filter,
        TopReceiver,
        Receiver
    };

public:
    HRESULT OnNotify(
        const std::wstring& strRegMsgId, 
        const std::wstring& strMsgId, 
        prg::IXParam *pParameter, 
        prg::IXParam *pSender, 
        bool &bFiltered)
    {            
        MessageSinkEntry entry;
        bool bFind = false;

        {
            base::AutoLock lk(m_mutexHandler);
            MsgHandlerMap::iterator it = m_mapHandler.find(strRegMsgId);
            if (it != m_mapHandler.end())
            {
                entry = (it->second);
                bFind = true;
            }
        }
            
        //回调代码不加锁
        if (bFind)
        {
            entry.Invoke(this, strMsgId, pParameter, pSender, bFiltered);
        }
            
        return S_OK;
    }

    HRESULT AddRegisterMessage(
        const std::wstring& strMsgId,
        MSGCENTER_PROC0 pfnProc,
        HandlerEntryType type = Receiver,
        const std::wstring& strMsgCenterName = L""
        )
    {
        base::AutoLock lk(m_mutexHandler);
        HRESULT hr = AddRegisterMessage(strMsgId, type, strMsgCenterName);
        if(hr == S_OK)
        {
            MessageSinkEntry entry;
            entry.strMsgCenterName = strMsgCenterName;
            entry.strMessageID = strMsgId;
            entry.functor.MsgProc_0 = pfnProc;
            entry.nMsgBusProcType = MessageSinkEntry::MsgCenterProcType0;
            m_mapHandler[entry.strMessageID] = entry;
        }

        return hr;
    }

    HRESULT AddRegisterMessage(
        const std::wstring& strMsgId,
        MSGCENTER_PROC1 pfnProc,
        HandlerEntryType type = Receiver,
        const std::wstring& strMsgCenterName = L""
        )   
    {
        base::AutoLock lk(m_mutexHandler);
        HRESULT hr = AddRegisterMessage(strMsgId, type, strMsgCenterName);

        if(hr == S_OK)
        {
            MessageSinkEntry entry;
            entry.strMsgCenterName = strMsgCenterName;
            entry.strMessageID = strMsgId;
            entry.functor.MsgProc_1 = pfnProc;
            entry.nMsgBusProcType = MessageSinkEntry::MsgCenterProcType1;
            m_mapHandler[entry.strMessageID] = entry;
        }
        return hr;
    }

    HRESULT AddRegisterMessage(
        const std::wstring& strMsgId,
        MSGCENTER_PROC2 pfnProc,
        HandlerEntryType type = Receiver,
        const std::wstring& strMsgCenterName = L""
        )
    {
        base::AutoLock lk(m_mutexHandler);
        HRESULT hr = AddRegisterMessage(strMsgId, type, strMsgCenterName);

        if(hr == S_OK)
        {
            MessageSinkEntry entry;
            entry.strMsgCenterName = strMsgCenterName;
            entry.strMessageID = strMsgId;
            entry.functor.MsgProc_2 = pfnProc;
            entry.nMsgBusProcType = MessageSinkEntry::MsgCenterProcType2;
            m_mapHandler[entry.strMessageID] = entry;
        }

        return hr;
    }

    HRESULT AddRegisterMessage(
        const std::wstring& strMsgId,
        MSGCENTER_PROC3 pfnProc,
        HandlerEntryType type = Receiver,
        const std::wstring& strMsgCenterName = L""
        )
    {
        base::AutoLock lk(m_mutexHandler);
        HRESULT hr = AddRegisterMessage(strMsgId, type, strMsgCenterName);

        if(hr == S_OK)
        {
            MessageSinkEntry entry;
            entry.strMsgCenterName = strMsgCenterName;
            entry.strMessageID = strMsgId;
            entry.functor.MsgProc_3 = pfnProc;
            entry.nMsgBusProcType = MessageSinkEntry::MsgCenterProcType3;
            m_mapHandler[entry.strMessageID] = entry;
        }

        return hr;
    }

    HRESULT AddRegisterMessage(
        const std::wstring& strMsgId,
        MSGCENTER_PROC4 pfnProc,
        HandlerEntryType type = Receiver,
        const std::wstring& strMsgCenterName = L""
        )
    {
        base::AutoLock lk(m_mutexHandler);
        HRESULT hr = AddRegisterMessage(strMsgId, type, strMsgCenterName);

        if(hr == S_OK)
        {
            MessageSinkEntry entry;
            entry.strMsgCenterName = strMsgCenterName;
            entry.strMessageID = strMsgId;
            entry.functor.MsgProc_4 = pfnProc;
            entry.nMsgBusProcType = MessageSinkEntry::MsgCenterProcType4;
            m_mapHandler[entry.strMessageID] = entry;
        }

        return hr;
    }
        
    HRESULT AddRegisterMessage_Fun0(
        const std::wstring& strMsgId,
        MSGCENTER_FUN0 fun,
        HandlerEntryType type = Receiver,
        const std::wstring& strMsgCenterName = L""
        )
    {
        base::AutoLock lk(m_mutexHandler);
        HRESULT hr = AddRegisterMessage(strMsgId, type, strMsgCenterName);

        if(hr == S_OK)
        {
            MessageSinkEntry entry;
            entry.strMsgCenterName = strMsgCenterName;
            entry.strMessageID = strMsgId;
            entry.functor.pMsgFun_0 = new MSGCENTER_FUN0(fun);
            entry.nMsgBusProcType = MessageSinkEntry::MsgCenterFunType0;
            m_mapHandler[entry.strMessageID] = entry;
        }

        return hr;
    }
        
    HRESULT AddRegisterMessage_Fun1(
        const std::wstring& strMsgId,
        MSGCENTER_FUN1 fun,
        HandlerEntryType type = Receiver,
        const std::wstring& strMsgCenterName = L""
        )
    {
        base::AutoLock lk(m_mutexHandler);
        HRESULT hr = AddRegisterMessage(strMsgId, type, strMsgCenterName);

        if(hr == S_OK)
        {
            MessageSinkEntry entry;
            entry.strMsgCenterName = strMsgCenterName;
            entry.strMessageID = strMsgId;
            entry.functor.pMsgFun_1 = new MSGCENTER_FUN1(fun);
            entry.nMsgBusProcType = MessageSinkEntry::MsgCenterFunType1;
            m_mapHandler[entry.strMessageID] = entry;
        }

        return hr;
    }

    HRESULT AddRegisterMessage_Fun2(
        const std::wstring& strMsgId,
        MSGCENTER_FUN2 fun,
        HandlerEntryType type = Receiver,
        const std::wstring& strMsgCenterName = L""
        )
    {
        base::AutoLock lk(m_mutexHandler);
        HRESULT hr = AddRegisterMessage(strMsgId, type, strMsgCenterName);

        if(hr == S_OK)
        {
            MessageSinkEntry entry;
            entry.strMsgCenterName = strMsgCenterName;
            entry.strMessageID = strMsgId;
            entry.functor.pMsgFun_2 = new MSGCENTER_FUN2(fun);
            entry.nMsgBusProcType = MessageSinkEntry::MsgCenterFunType2;
            m_mapHandler[entry.strMessageID] = entry;
        }

        return hr;
    }

    HRESULT AddRegisterMessage_Fun3(
        const std::wstring& strMsgId,
        MSGCENTER_FUN3 fun,
        HandlerEntryType type = Receiver,
        const std::wstring& strMsgCenterName = L""
        )
    {
        base::AutoLock lk(m_mutexHandler);
        HRESULT hr = AddRegisterMessage(strMsgId, type, strMsgCenterName);

        if(hr == S_OK)
        {
            MessageSinkEntry entry;
            entry.strMsgCenterName = strMsgCenterName;
            entry.strMessageID = strMsgId;
            entry.functor.pMsgFun_3 = new MSGCENTER_FUN3(fun);
            entry.nMsgBusProcType = MessageSinkEntry::MsgCenterFunType3;
            m_mapHandler[entry.strMessageID] = entry;
        }

        return hr;
    }

    HRESULT AddRegisterMessage_Fun4(
        const std::wstring& strMsgId,
        MSGCENTER_FUN4 fun,
        HandlerEntryType type = Receiver,
        const std::wstring& strMsgCenterName = L""
        )
    {
        base::AutoLock lk(m_mutexHandler);
        HRESULT hr = AddRegisterMessage(strMsgId, type, strMsgCenterName);

        if(hr == S_OK)
        {
            MessageSinkEntry entry;
            entry.strMsgCenterName = strMsgCenterName;
            entry.strMessageID = strMsgId;
            entry.functor.pMsgFun_4 = new MSGCENTER_FUN4(fun);
            entry.nMsgBusProcType = MessageSinkEntry::MsgCenterFunType4;
            m_mapHandler[entry.strMessageID] = entry;
        }

        return hr;
    }

    virtual HRESULT RemoveRegisteredMessage(const std::wstring& strMsgId)
    {
        std::wstring name = L"";
        {
            base::AutoLock lk(m_mutexHandler);
            MsgHandlerMap::iterator it = m_mapHandler.find(strMsgId);
            if (it == m_mapHandler.end())
            {
                return S_FALSE;
            }
            name = it->second.strMsgCenterName;
            m_mapHandler.erase(it);
        }
            
        IPrgMsgHandler *pMsgHandler = GetMsgHandler();
        HRESULT hr = prg::msgcenter(name).UnRegisterMessage(strMsgId, pMsgHandler);
        return hr;
    }

    virtual HRESULT RemoveAllRegisteredMessage()
    {
        base::AutoLock lk(m_mutexHandler);

        IPrgMsgHandler *pMsgHandler = GetMsgHandler();

        MsgHandlerMap::iterator it = m_mapHandler.begin();

        for (; it != m_mapHandler.end(); ++it)
        {
            prg::msgcenter(it->second.strMsgCenterName).UnRegisterMessage(it->first.c_str(), pMsgHandler);
        }

        m_mapHandler.clear();
        return S_OK;
    }
    
protected:
    HRESULT AddRegisterMessage(
        const std::wstring& strMsgId,
        HandlerEntryType type = Receiver,
        const std::wstring& strMsgCenterName = L""
        )
    {
        bool bFind = false;

        {
            MsgHandlerMap::iterator it = m_mapHandler.find(strMsgId);
            if (it != m_mapHandler.end())
            {
                bFind = true;
            }
        }

        if (bFind)
        {
            return S_FALSE;
        }
            
        HRESULT hr = S_OK;
        {
            IPrgMsgHandler *pMsgHandler = GetMsgHandler();

            switch(type)
            {
            case TopFilter:
                hr = prg::msgcenter(strMsgCenterName).RegisterMessage(strMsgId, pMsgHandler, prg::REGISTERTYPE_TOPFILTER);
                break;
            case Filter:
                hr = prg::msgcenter(strMsgCenterName).RegisterMessage(strMsgId, pMsgHandler, prg::REGISTERTYPE_FILTER);                
                break;
            case TopReceiver:
                hr = prg::msgcenter(strMsgCenterName).RegisterMessage(strMsgId, pMsgHandler, prg::REGISTERTYPE_TOPRECEIVER);                
                break;
            case Receiver:
                hr = prg::msgcenter(strMsgCenterName).RegisterMessage(strMsgId, pMsgHandler, prg::REGISTERTYPE_RECEIVER);                
                break;
            default:
                assert(0);
                return E_UNEXPECTED;
            }
        }

        return hr;
    }

protected:
  base::Lock m_mutexHandler;
    MsgHandlerMap m_mapHandler;
};

class CMsgCenterRegHandler : public CMsgCenterRegHandlerBase
{
public:
    virtual ULONG REFCALLTYPE AddRef()
    {
        return CMsgCenterRegHandlerBase::InternalAddRef();
    }

    virtual ULONG REFCALLTYPE Release()
    {
		ULONG res = CMsgCenterRegHandlerBase::InternalRelease();
        if (res == 0)
        {
            delete this;
			return 0;
        }
		return res;
    }

    virtual HRESULT QueryInterface(const std::string &iid, void** ppvObject)
    {
        if (ppvObject == NULL)
        {
            return E_INVALIDARG;
        }
        if (iid == i_uuidof(IPrgMsgHandler))
        {
            IPrgMsgHandler* pMsgHandler = dynamic_cast<IPrgMsgHandler*>(this);
            if (pMsgHandler != NULL)
            {
                pMsgHandler->AddRef();
                *ppvObject = pMsgHandler;
                return S_OK;
            }
        }

        return E_PRGCORE_NOINTERFACE; 
    }
};

END_NAMESPACE()

#define DECLARE_MSGCENTER_MAP()               \
    virtual void RegisterMessageCenter(); 
    

#define BEGIN_MSGCENTER_MAP(T)                 \
    typedef T __MSGCENTER_FUNCTOR_CLASS;       \
     void T::RegisterMessageCenter()           \
    {                                           

#define END_MSGCENTER_MAP()                    \
    }

#define CAST_MSGCENTER_PROC0(func)  static_cast<prg::MSGCENTER_PROC0>(func)
#define CAST_MSGCENTER_PROC1(func)  static_cast<prg::MSGCENTER_PROC1>(func)
#define CAST_MSGCENTER_PROC2(func)  static_cast<prg::MSGCENTER_PROC2>(func)
#define CAST_MSGCENTER_PROC3(func)  static_cast<prg::MSGCENTER_PROC3>(func)
#define CAST_MSGCENTER_PROC4(func)  static_cast<prg::MSGCENTER_PROC4>(func)

#define REG_MSGCENTER_PROC0(msid, fun, handler_type, msgcenter)     \
    AddRegisterMessage(msid, \
    CAST_MSGCENTER_PROC0(&__MSGCENTER_FUNCTOR_CLASS::fun), handler_type, msgcenter);

#define REG_MSGCENTER_PROC1(msid, fun, handler_type, msgcenter) \
    AddRegisterMessage(msid, \
    CAST_MSGCENTER_PROC1(&__MSGCENTER_FUNCTOR_CLASS::fun), handler_type, msgcenter);

#define REG_MSGCENTER_PROC2(msid, fun, handler_type, msgcenter) \
    AddRegisterMessage(msid, \
    CAST_MSGCENTER_PROC2(&__MSGCENTER_FUNCTOR_CLASS::fun), handler_type, msgcenter);

#define REG_MSGCENTER_PROC3(msid, fun, handler_type, msgcenter) \
    AddRegisterMessage(msid, \
    CAST_MSGCENTER_PROC3(&__MSGCENTER_FUNCTOR_CLASS::fun), handler_type, msgcenter);

#define REG_MSGCENTER_PROC4(msid, fun, handler_type, msgcenter) \
    AddRegisterMessage(msid, \
    CAST_MSGCENTER_PROC4(&__MSGCENTER_FUNCTOR_CLASS::fun), handler_type, msgcenter);

#define REG_MSGCENTER_FUN0(msid, fun, handler_type, msgcenter) \
    AddRegisterMessage_Fun0(msid, \
    fun, handler_type, msgcenter);

#define REG_MSGCENTER_FUN1(msid, fun, handler_type, msgcenter) \
    AddRegisterMessage_Fun1(msid, \
    fun, handler_type, msgcenter);

#define REG_MSGCENTER_FUN2(msid, fun, handler_type, msgcenter) \
    AddRegisterMessage_Fun2(msid, \
    fun, handler_type, msgcenter);

#define REG_MSGCENTER_FUN3(msid, fun, handler_type, msgcenter) \
    AddRegisterMessage_Fun3(msid, \
    fun, handler_type, msgcenter);

#define REG_MSGCENTER_FUN4(msid, fun, handler_type, msgcenter) \
    AddRegisterMessage_Fun4(msid, \
    fun, handler_type, msgcenter);

#define BASE_CLASS_MSGCENTER_MAP(base)      base::RegisterMessageCenter();

#endif // __MSGCENTER_REGISTRATION_H_1610FB0C_19A4_4C37_B7C6_82E61360C200_INCLUDED__