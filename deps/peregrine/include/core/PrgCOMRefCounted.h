//------------------------------------------------------------------------------
//
//    版权所有(C) 阿里巴巴（中国）网络技术有限公司 保留所有权利
//
//    创建者:   徐明
//    创建日期: 2015-01-29
//    功能描述: PrgCOM类接口的引用计数实现，具有线程安全，非线程安全的实现和支持弱引用计数的线程安全实现
//
//
//------------------------------------------------------------------------------

#ifndef __COMREFCOUNTED_H_A7FD4DC8_2445_49C6_AE6F_2B6CF296A471_INCLUDED__
#define __COMREFCOUNTED_H_A7FD4DC8_2445_49C6_AE6F_2B6CF296A471_INCLUDED__

#include "base/namespace.h"
#include "core/PrgCore.h"
#include <cassert>
#include "base/atomic_ref_count.h"
#include "base/atomic_weak_ref_count.h"
#include "base/base_export.h"
#include "base/compiler_specific.h"
#include "base/threading/thread_collision_warner.h"

BEGIN_NAMESPACE(prg)

class CCOMThreadNoSafeRefPolicy {
public:
	bool HasOneRef() const { return ref_count_ == 1; }

	virtual ULONG InternalAddRef() const
	{
		return ++ref_count_;
	}

	virtual ULONG InternalRelease() const
	{
		return --ref_count_;
	}

protected:
	CCOMThreadNoSafeRefPolicy(): ref_count_(0)
	{
	}

	virtual ~CCOMThreadNoSafeRefPolicy()
	{
	}

private:
	mutable int ref_count_;

	DISALLOW_COPY_AND_ASSIGN(CCOMThreadNoSafeRefPolicy);
};

class CCOMThreadSafeRefPolicy {
public:
	bool HasOneRef() const
	{
		return base::AtomicRefCountIsOne(
			&const_cast<CCOMThreadSafeRefPolicy*>(this)->ref_count_);
	}

	virtual ULONG InternalAddRef() const
	{
		return base::AtomicRefCountInc(&ref_count_);
	}

	virtual ULONG InternalRelease() const
	{
		return base::AtomicRefCountDecEx(&ref_count_);
	}

protected:
	CCOMThreadSafeRefPolicy():ref_count_(0)
	{
	}

	virtual ~CCOMThreadSafeRefPolicy()
	{
	}

private:
	mutable base::AtomicRefCount ref_count_;

	DISALLOW_COPY_AND_ASSIGN(CCOMThreadSafeRefPolicy);
};

class CCOMThreadSafeWeakRefPolicy {
public:
    bool HasOneRef() const
    {
        return weak_ref_count_->AtomicIsOne();
    }

    virtual ULONG InternalAddRef() const
    {
        return weak_ref_count_->AtomicIncrease();
    }

    virtual ULONG InternalRelease() const
    {
        return weak_ref_count_->AtomicDecrease();
    }

protected:
    CCOMThreadSafeWeakRefPolicy():weak_ref_count_(0)
    {
    }

    virtual ~CCOMThreadSafeWeakRefPolicy()
    {
    }
    mutable prg::AtomicWeakRefCount* weak_ref_count_;

private:
    
    DISALLOW_COPY_AND_ASSIGN(CCOMThreadSafeWeakRefPolicy);
};

template <class ThreadModel>
class CPrgCOMRefCounted: public ThreadModel
{
public:
	virtual ULONG InternalAddRef() const {
		return ThreadModel::InternalAddRef();
	}

	virtual ULONG InternalRelease() const {
		return ThreadModel::InternalRelease();
	}

protected:
	CPrgCOMRefCounted() {}
	virtual ~CPrgCOMRefCounted() {}

private:
	DISALLOW_COPY_AND_ASSIGN(CPrgCOMRefCounted<ThreadModel>);
};

END_NAMESPACE()

#endif  // __COMREFCOUNTED_H_A7FD4DC8_2445_49C6_AE6F_2B6CF296A471_INCLUDED__
