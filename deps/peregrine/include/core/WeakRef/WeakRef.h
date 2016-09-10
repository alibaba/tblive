//------------------------------------------------------------------------------
//
//  版权所有(C) 浙江天猫技术有限公司 保留所有权利
//
//  创建者:   哈尔
//  创建日期: 2015/02/15
//  功能描述: 弱引用对象实现
//
//------------------------------------------------------------------------------

#ifndef __WEAKREF_H_10D2DF38_DFAE_4CD2_BED9_C6162E16EC00_INCLUDED__
#define __WEAKREF_H_10D2DF38_DFAE_4CD2_BED9_C6162E16EC00_INCLUDED__

#include <core/IWeakRef.h>
#include <core/ISupportWeakRef.h>
#include <base/atomic_weak_ref_count.h>
#include <base/memory/ref_ptrcounted.h>

class CWeakRef : public base::RefPtrCounted<prg::IWeakRef, base::ThreadSafeRefPolicy>
{
public:
    CWeakRef() : m_pSupportWeakRef(NULL) {}

    virtual HRESULT BindStrongRef(ISupportWeakRef* pSupportWeakRef, prg::AtomicWeakRefCount** ppRefCount) OVERRIDE;

    virtual HRESULT GetStrongRef(const std::string& iid, void** ppObject) OVERRIDE;

private:
    ISupportWeakRef* m_pSupportWeakRef;
    prg::AtomicWeakRefCount m_AtomicRefCount;
};

#endif // __WEAKREF_H_10D2DF38_DFAE_4CD2_BED9_C6162E16EC00_INCLUDED__
