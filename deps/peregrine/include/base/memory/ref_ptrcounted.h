// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MEMORY_REFPTRCOUNTED_COUNTED_H_
#define BASE_MEMORY_REFPTRCOUNTED_COUNTED_H_

#include <cassert>
#include "base/basictypes.h"
#include "base/atomic_ref_count.h"
#include "base/compiler_specific.h"
#include "base/threading/thread_collision_warner.h"
#include "iref_counted.h"

namespace base {
	class ThreadNoSafeRefPolicy {
	public:
		bool HasOneRef()  { return ref_count_ == 1; }

		ULONG AddRef()
		{
			return (ULONG)++ref_count_;
		}

		// Returns true if the object should self-delete.
		ULONG Release()
		{
			return (ULONG)--ref_count_;
		}

	protected:
		ThreadNoSafeRefPolicy(): ref_count_(0)
		{
		}

		virtual ~ThreadNoSafeRefPolicy()
		{
		}

	private:
		mutable int ref_count_;

		DISALLOW_COPY_AND_ASSIGN(ThreadNoSafeRefPolicy);
	};

	class ThreadSafeRefPolicy {
	public:
		bool HasOneRef()
		{
			return AtomicRefCountIsOne(
				&const_cast<ThreadSafeRefPolicy*>(this)->ref_count_);
		}

		ULONG AddRef()
		{
			return (ULONG)AtomicRefCountInc(&ref_count_);
		}

		// Returns true if the object should self-delete.
		ULONG Release()
		{
			return (ULONG)AtomicRefCountDecEx(&ref_count_);
		}

	protected:
		ThreadSafeRefPolicy():ref_count_(0)
		{
		}

		virtual ~ThreadSafeRefPolicy()
		{
		}

	private:
		mutable AtomicRefCount ref_count_;

		DISALLOW_COPY_AND_ASSIGN(ThreadSafeRefPolicy);
	};

	template <class Interface, class ThreadModel>
	class RefPtrCounted : public Interface, public ThreadModel
	{
	public:
		typedef RefPtrCounted<Interface, ThreadModel> ThisRefPtrCounted;
		
		virtual ULONG REFCALLTYPE AddRef() {
			return ThreadModel::AddRef();
		}

		virtual ULONG REFCALLTYPE Release() {
			ULONG refCount = ThreadModel::Release();
			if (refCount == 0) {
				delete this;
				return 0;
			}
			return refCount;
		}

	protected:
		RefPtrCounted() {}
		virtual ~RefPtrCounted() {}

	private:
		DISALLOW_COPY_AND_ASSIGN(RefPtrCounted);
	};
}

#endif
