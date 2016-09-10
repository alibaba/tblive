//------------------------------------------------------------------------------
//
//  版权所有(C) 浙江天猫技术有限公司 保留所有权利
//
//  创建者:   哈尔
//  创建日期: 2015/02/15
//  功能描述: 消息观察者回调接口定义
//
//------------------------------------------------------------------------------

#ifndef __IPRGMSGHANDLER_H_D00461C6_D60D_4424_BF77_1512B25665BA_INCLUDED__
#define __IPRGMSGHANDLER_H_D00461C6_D60D_4424_BF77_1512B25665BA_INCLUDED__

#include <string>
#include <base/namespace.h>
#include <base/xparam/IXParam.h>
#include <core/ISupportWeakRef.h>

class IPrgMsgHandler : public ISupportWeakRef
{
public:
    /**
    * @brief 观察者的回调接口
    * @param strRegMsg 注册时的消息 
    * @param strMsg 主题消息
    * @param pParam 万能参数
    * @param pSender 主题发送时的Sender描述
    * @param bFilter 过滤器时生效，表明是否继续把主题消息路由
    * @return S_OK
    * @remark 
    */
    virtual HRESULT OnNotify(const std::wstring& strRegMsg, const std::wstring& strMsg, prg::IXParam *pParam, prg::IXParam *pSender, bool &bFilter) = 0;
};

DEFINE_IID(IPrgMsgHandler, "{6AB5D334-BC88-476D-97BD-A5A2C776EE19}")

#endif // __IPRGMSGHANDLER_H_D00461C6_D60D_4424_BF77_1512B25665BA_INCLUDED__
