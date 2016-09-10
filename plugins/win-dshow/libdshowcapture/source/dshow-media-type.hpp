/*
 *  Copyright (C) 2014 Hugh Bailey <obs.jim@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 */

#pragma once

#include "dshow-base.hpp"

namespace DShow {

HRESULT CopyMediaType(AM_MEDIA_TYPE *pmtTarget, const AM_MEDIA_TYPE *pmtSource);
void    FreeMediaType(AM_MEDIA_TYPE &mt);

BITMAPINFOHEADER *GetBitmapInfoHeader(AM_MEDIA_TYPE &mt);
const BITMAPINFOHEADER *GetBitmapInfoHeader(const AM_MEDIA_TYPE &mt);

class MediaTypePtr;

class MediaType {
	friend class MediaTypePtr;

	AM_MEDIA_TYPE type;

public:
	inline MediaType() {memset(&type, 0, sizeof(type));}

	inline MediaType(const MediaType &mt)
	{
		CopyMediaType(&type, &mt.type);
	}

	inline MediaType(const AM_MEDIA_TYPE &type_)
	{
		CopyMediaType(&type, &type_);
	}

	inline ~MediaType() {FreeMediaType(type);}

	inline operator AM_MEDIA_TYPE*()             {return &type;}
	inline operator AM_MEDIA_TYPE&()             {return type;}
	inline operator const AM_MEDIA_TYPE*() const {return &type;}
	inline operator const AM_MEDIA_TYPE&() const {return type;}
	inline AM_MEDIA_TYPE *Ptr()                  {return &type;}

	inline AM_MEDIA_TYPE *operator->()           {return &type;}

	inline AM_MEDIA_TYPE *Duplicate() const
	{
		AM_MEDIA_TYPE *ptr =
			(AM_MEDIA_TYPE*)CoTaskMemAlloc(sizeof(*ptr));
		memset(ptr, 0, sizeof(*ptr));
		CopyMediaType(ptr, &type);
		return ptr;
	}

	inline bool operator==(const AM_MEDIA_TYPE *pMT) const
	{
		return pMT == &type;
	}

	inline void operator=(const MediaType &mt)
	{
		FreeMediaType(type);
		CopyMediaType(&type, &mt.type);
	}

	inline void operator=(const AM_MEDIA_TYPE *pMT)
	{
		FreeMediaType(type);
		CopyMediaType(&type, pMT);
	}

	inline void operator=(const AM_MEDIA_TYPE &type_)
	{
		FreeMediaType(type);
		CopyMediaType(&type, &type_);
	}

	template <typename T> inline T *AllocFormat()
	{
		if (type.pbFormat) {
			CoTaskMemFree(type.pbFormat);
			type.pbFormat = nullptr;
			type.cbFormat = 0;
		}

		type.pbFormat = (PBYTE)CoTaskMemAlloc(sizeof(T));
		type.cbFormat = sizeof(T);
		memset(type.pbFormat, 0, sizeof(T));

		return (T*)type.pbFormat;
	}
};

class MediaTypePtr {
	friend class MediaType;

	AM_MEDIA_TYPE *ptr;

public:
	inline void Clear()
	{
		if (ptr) {
			FreeMediaType(*ptr);
			CoTaskMemFree(ptr);
			ptr = nullptr;
		}
	}

	inline MediaTypePtr() : ptr(nullptr) {}
	inline MediaTypePtr(AM_MEDIA_TYPE *ptr_) : ptr(ptr_) {}
	inline ~MediaTypePtr() {Clear();}

	inline AM_MEDIA_TYPE **operator&() {Clear(); return &ptr;}

	inline AM_MEDIA_TYPE *operator->() const {return ptr;}

	inline operator AM_MEDIA_TYPE*() const {return ptr;}

	inline void operator=(AM_MEDIA_TYPE *ptr_) {Clear(); ptr = ptr_;}

	inline bool operator==(const AM_MEDIA_TYPE *ptr_) const
	{
		return ptr == ptr_;
	}
};

}; /* namespace DShow */
