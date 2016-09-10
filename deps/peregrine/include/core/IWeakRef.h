//------------------------------------------------------------------------------
//
//  版权所有(C) 浙江天猫技术有限公司 保留所有权利
//
//  创建者:   哈尔
//  创建日期: 2015/02/15
//  功能描述: 弱引用对象接口定义
//
//------------------------------------------------------------------------------

#ifndef __IWEAKREF_H_00E00424_8E9C_4251_B53F_E87E70171FF4_INCLUDED__
#define __IWEAKREF_H_00E00424_8E9C_4251_B53F_E87E70171FF4_INCLUDED__

#include <base/namespace.h>
#include <base/memory/iref_counted.h>
#include <string>

class ISupportWeakRef;

BEGIN_NAMESPACE(prg)

class AtomicWeakRefCount;

class IWeakRef : public base::IRefCounted
{
public:
    /**
    * @brief 与强引用指针进行绑定
    * @param pSupportWeakRef 绑定的强引用对象
    * @param ppRefCount 用来获取引用计数对象地址的二级指针
    * @return hresult_define.h
    * @remark 
    */
    virtual HRESULT BindStrongRef(ISupportWeakRef* pSupportWeakRef, prg::AtomicWeakRefCount** ppRefCount) = 0;
    
    /**
    * @brief 获取强引用
    * @param iid 强引用接口ID
    * @param ppvObject 强引用对象
    * @return 成功获取返回S_OK，失败返回hresult_define.h
    * @remark 
    */
    virtual HRESULT GetStrongRef(const std::string& iid, void** ppObject) = 0;
};

END_NAMESPACE()

#endif // __IWEAKREF_H_00E00424_8E9C_4251_B53F_E87E70171FF4_INCLUDED__
