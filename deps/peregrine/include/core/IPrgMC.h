//------------------------------------------------------------------------------
//
//    版权所有(C) 阿里巴巴（中国）网络技术有限公司 保留所有权利
//
//    创建者:   徐明
//    创建日期: 2011-11-07
//    功能描述: PrgCore底下的消息中心
//    为了解决观察者模式中，观察者必须掌握主题的所在地点。
//    (1)MessageCenter为此上面两者解耦，观察者在消息中心注册消息，主题往消息中心  
//    抛售消息，由消息中心挂接两者的关系。
//    (2)消息中心现包含同步模式和异步模式
//    同步模式：主题发送者通过SendMessage同步进入消息中心，
//    只有当观察者的接口被回调完毕，SendMessage才return，接口回调和SendMessage处于
//    同一线程。
//    异步模式：主题发送者通过PostMessage异步进入消息中心，PostMessage立马返回  
//    此时由异步模式中的线程池，交替的获取Message，并回调观察者接口。
//    (3)PostMessage支持不同的消息等级，异步模式尽可能的保证高优先级的消息尽早处理。
//    (4)观察者注册消息，注册消息可以分为高级过滤器，过滤器，高级接收者，接收者，
//    其中回调按上面的顺序依次调用，过滤器可以使消息不再往底下回调，即吞掉消息。
//    (5)如果注册a.b.消息，同时主题发送a.b.c，则观察者能正常接收
//    (6)反初始化接口，会等待所有的异步线程池和同步接口回调全部完成后才行返回
//    不要在回调接口中调用UnInit，Init和UnInit配对调用
//------------------------------------------------------------------------------


/**
* @class IPrgMC
* @brief 为了解决观察者模式中，观察者必须掌握主题的所在地点。
* (1)MessageCenter为此上面两者解耦，观察者在消息中心注册消息，主题往消息中心  
* 抛售消息，由消息中心挂接两者的关系。
* (2)消息中心现包含同步模式和异步模式
* 同步模式：主题发送者通过SendMessage同步进入消息中心，
* 只有当观察者的接口被回调完毕，SendMessage才return，接口回调和SendMessage处于
* 同一线程。
* 异步模式：主题发送者通过PostMessage异步进入消息中心，PostMessage立马返回  
* 此时由异步模式中的线程池，交替的获取Message，并回调观察者接口。
* (3)PostMessage支持不同的消息等级，异步模式尽可能的保证高优先级的消息尽早处理。
* (4)观察者注册消息，注册消息可以分为高级过滤器，过滤器，高级接收者，接收者，
* 其中回调按上面的顺序依次调用，过滤器可以使消息不再往底下回调，即吞掉消息。
* (5)如果注册a.b.消息，同时主题发送a.b.c，则观察者能正常接收
* (6)反初始化接口，会等待所有的异步线程池和同步接口回调全部完成后才行返回
* 不要在回调接口中调用UnInit，Init和UnInit配对调用
* @author 徐明
* @version 1.0.0
*/

#ifndef __IPRGMC_H_72EAAE37_FFB6_41FA_A34E_1FD14A90FA47_INCLUDED__
#define __IPRGMC_H_72EAAE37_FFB6_41FA_A34E_1FD14A90FA47_INCLUDED__

#include <string>
#include <base/memory/iref_counted.h>
#include <base/namespace.h>

class IPrgMsgHandler;

BEGIN_NAMESPACE(prg)

/*!
*  @defgroup RVMessageCenter
*  @brief RVMessageCenter解决方案
*  @{
*/
enum REGISTERTYPE
{
    REGISTERTYPE_TOPFILTER,
    REGISTERTYPE_FILTER,
    REGISTERTYPE_TOPRECEIVER,
    REGISTERTYPE_RECEIVER
};

enum ASYNCPRIORITYTYPE
{
    ASYNCPRIORITYTYPE_TOP,
    ASYNCPRIORITYTYPE_NORMAL,
    ASYNCPRIORITYTYPE_IDLE
};

class IXParam;

class IPrgMC:public base::IRefCounted
{
public:
    /**
    * @brief 初始化
    * @param nAsyncMaxThread 异步消息中心线程池最大线程数，线程用时创建。
    * @return S_OK
    * @remark 
    */
    virtual HRESULT Init(int nAsyncMaxThread = 1) = 0;
    
    /**
    * @brief 反初始化
    * @return S_OK
    * @remark 为保证异步线程池正确关闭，需要在调用Init的线程中调用Uninit。
    */
    virtual HRESULT UnInit() = 0;
    
    /**
    * @brief 注册消息
    * @param lpszMsg 消息名称
    * @param pHandler 回调接口
    * @param type 接收时类型，如过滤器，接收器
    * @return 如果未初始化，返回E_CORE_RVMC_UNINIT，否则S_OK，
    * @remark 见上面描述
    */
    virtual HRESULT RegisterMessage(const std::wstring& strMsg, IPrgMsgHandler *pHandler, REGISTERTYPE type) = 0;
    
    /**
    * @brief 反注册消息
    * @param lpszMsg 消息名称
    * @param pHandler 回调接口
    * @return 如果未初始化，返回E_CORE_RVMC_UNINIT，否则S_OK，
    * @remark 见上面描述
    */
    virtual HRESULT UnRegisterMessage(const std::wstring& strMsg, IPrgMsgHandler *pHandler) = 0;
    
    /**
    * @brief 同步发送消息
    * @param lpszMsg 消息名称
    * @param pParam 主题参数
    * @param pSender 主题Sender
    * @return 如果未初始化，返回E_CORE_RVMC_UNINIT，否则S_OK，
    * @remark 见上面描述
    */
    virtual HRESULT SendMessage(const std::wstring& strMsg, prg::IXParam *pParam, prg::IXParam *pSender) = 0;
    
    /**
    * @brief 异步发送消息
    * @param lpszMsg 消息名称
    * @param pParam 主题参数
    * @param pSender 主题Sender
    * @param type 消息的优先级
    * @return 如果未初始化，返回E_CORE_RVMC_UNINIT，否则S_OK，
    * @remark 见上面描述
    */
    virtual HRESULT PostMessage(const std::wstring& strMsg, prg::IXParam *pParam, prg::IXParam *pSender, ASYNCPRIORITYTYPE type) = 0;

    /**
    * @brief 注册全局的Filter，此消息中心所有消息都优先过滤
    * @param pHandler 回调接口
    * @return 如果未初始化，返回E_CORE_RVMC_UNINIT，否则S_OK，
    * @remark 见上面描述
    */
    virtual HRESULT RegisterGlobalFilter(IPrgMsgHandler *pHandler) = 0;
    
    /**
    * @brief 反注册全局的Filter 
    * @param pHandler 回调接口
    * @return 如果未初始化，返回E_CORE_RVMC_UNINIT，否则S_OK，
    * @remark 见上面描述
    */
    virtual HRESULT UnRegisterGlobalFilter(IPrgMsgHandler *pHandler) = 0;

    /**
    * @brief 清空PostMessage的队列中的msgs
    * @return 如果未初始化，返回E_CORE_RVMC_UNINIT，否则S_OK，
    * @remark 见上面描述
    */
    virtual HRESULT ClearPostMessages() = 0;

};
/** @} */ // end of PrgMessageCenter

END_NAMESPACE()



#endif // #ifndef __IPRGMC_H_72EAAE37_FFB6_41FA_A34E_1FD14A90FA47_INCLUDED__
