//------------------------------------------------------------------------------
//
//  版权所有(C) 浙江天猫技术有限公司 保留所有权利
//
//  创建者:   哈尔
//  创建日期: 2015/02/15
//  功能描述: 弱引用支持接口定义
//
//------------------------------------------------------------------------------

#ifndef __ISUPPORTWEAKREF_H_660D9BE2_DA67_46C4_BB5A_DF36AE7CA09C_INCLUDED__
#define __ISUPPORTWEAKREF_H_660D9BE2_DA67_46C4_BB5A_DF36AE7CA09C_INCLUDED__

#include <base/namespace.h>
#include <core/IWeakRef.h>
#include <core/IPrgCOMRefCounted.h>

class ISupportWeakRef : public prg::IPrgCOMRefCounted
{
public:
    /**
    * @brief 获取弱引用指针
    * @param ppWeakRef 弱引用指针出参
    * @return S_OK
    * @remark 
    */
    virtual HRESULT GetWeakRef(prg::IWeakRef **ppWeakRef) = 0;

    /**
    * @brief 获取指定接口的强引用指针
    * @param iid 指定强引用接口
    * @param ppObject 强引用指针出参
    * @return hresult_define.h
    * @remark 
    */
    virtual HRESULT GetStrongRef(const std::string &iid, void **ppObject) = 0;
};

DEFINE_IID(ISupportWeakRef, "{82FBE323-E2F9-442F-9FD7-2628A1D7591A}")

#endif // __ISUPPORTWEAKREF_H_660D9BE2_DA67_46C4_BB5A_DF36AE7CA09C_INCLUDED__
