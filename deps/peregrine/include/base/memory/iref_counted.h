// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MEMORY_IREF_COUNTED_H_
#define BASE_MEMORY_IREF_COUNTED_H_

//就是为了兼容IUnknown服务的
#ifdef WIN32
	#define REFCALLTYPE __stdcall
#else
	#define REFCALLTYPE
#endif

#ifndef ULONG
    typedef unsigned long ULONG;
#endif

namespace base {
	class IRefCounted
	{
	public:
		virtual ULONG REFCALLTYPE AddRef() = 0;
		virtual ULONG REFCALLTYPE Release()  = 0;
	};

	class IRefCountedNative
	{
	public:
		virtual void AddRef() = 0;
		virtual void Release() = 0;
	};
}

#define DECLARE_SINGLETON_NATIVEREFCOUNT()\
	virtual void AddRef() {}\
	virtual void Release() {}

#endif  // BASE_MEMORY_IREF_COUNTED_H_
