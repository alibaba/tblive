#pragma once
#include "base/atomic_ref_count.h"
#include "base/memory/ref_counted.h"

namespace base
{
	class ThreadSafeAtomicRefCount:public base::RefCountedThreadSafe<ThreadSafeAtomicRefCount>
	{
	public:
		ThreadSafeAtomicRefCount():ref_count_(0)
		{
		}

		void RefCountInc()
		{
			base::AtomicRefCountInc(&ref_count_);
		}

		void RefCountDec()
		{
			base::AtomicRefCountDec(&ref_count_);
		}

		bool RefCountIsZero()
		{
			return AtomicRefCountIsZero(&ref_count_);
		}

		int GetRefCount()
		{
			return ref_count_;
		}

	public:
		volatile base::AtomicRefCount ref_count_;
	};
}