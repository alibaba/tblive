//------------------------------------------------------------------------------
//
//  版权所有(C) 浙江天猫技术有限公司 保留所有权利
//
//  创建者:   哈尔
//  创建日期: 2015/02/15
//  功能描述: 弱引用指针引用计数实现
//
//------------------------------------------------------------------------------

#ifndef __ATOMICWEAKREFCOUNT_H_2B052696_E675_470C_BEC2_7176EAB122AD_INCLUDED__
#define __ATOMICWEAKREFCOUNT_H_2B052696_E675_470C_BEC2_7176EAB122AD_INCLUDED__

#include "base/synchronization/lock.h"

BEGIN_NAMESPACE(prg)

class AtomicWeakRefCount
{
public:
    AtomicWeakRefCount() : m_refCount(0) {}

    ULONG AtomicIncrease()
    {
        base::AutoLock lk(m_lock);

        return ++m_refCount;
    }

    ULONG AtomicDecrease()
    {
        base::AutoLock lk(m_lock);

        return --m_refCount;
    }

    bool AtomicIsOne()
    {
        base::AutoLock lk(m_lock);

        return m_refCount == 1;
    }

    bool AtomicStrongIncrease()
    {
        base::AutoLock lk(m_lock);

        if (m_refCount > 0)
        {
            m_refCount++;
            return true;
        }
        else
        {
            return false;
        }
    }

private:
    base::Lock m_lock;
    volatile long m_refCount;
};

END_NAMESPACE()

#endif // __ATOMICWEAKREFCOUNT_H_2B052696_E675_470C_BEC2_7176EAB122AD_INCLUDED__
