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

#include "dshow-media-type.hpp"

namespace DShow {

HRESULT CopyMediaType(AM_MEDIA_TYPE *pmtTarget, const AM_MEDIA_TYPE *pmtSource)
{
	if (!pmtSource || !pmtTarget)
		return S_FALSE;

	*pmtTarget = *pmtSource;

	if (pmtSource->cbFormat && pmtSource->pbFormat) {
		pmtTarget->pbFormat =
			(PBYTE)CoTaskMemAlloc(pmtSource->cbFormat);

		if (pmtTarget->pbFormat == nullptr) {
			pmtTarget->cbFormat = 0;
			return E_OUTOFMEMORY;
		} else {
			memcpy(pmtTarget->pbFormat, pmtSource->pbFormat,
					pmtTarget->cbFormat);
		}
	}

	if(pmtTarget->pUnk != nullptr)
		pmtTarget->pUnk->AddRef();

	return S_OK;
}

void FreeMediaType(AM_MEDIA_TYPE &mt)
{
	if(mt.cbFormat != 0) {
		CoTaskMemFree((LPVOID)mt.pbFormat);
		mt.cbFormat = 0;
		mt.pbFormat = nullptr;
	}

	if (mt.pUnk) {
		mt.pUnk->Release();
		mt.pUnk = nullptr;
	}
}

BITMAPINFOHEADER *GetBitmapInfoHeader(AM_MEDIA_TYPE &mt)
{
	if (mt.formattype == FORMAT_VideoInfo) {
		VIDEOINFOHEADER *vih;
		vih = reinterpret_cast<VIDEOINFOHEADER*>(mt.pbFormat);
		return &vih->bmiHeader;

	} else if (mt.formattype == FORMAT_VideoInfo2) {
		VIDEOINFOHEADER2 *vih;
		vih = reinterpret_cast<VIDEOINFOHEADER2*>(mt.pbFormat);
		return &vih->bmiHeader;
	}

	return NULL;
}

const BITMAPINFOHEADER *GetBitmapInfoHeader(const AM_MEDIA_TYPE &mt)
{
	if (mt.formattype == FORMAT_VideoInfo) {
		const VIDEOINFOHEADER *vih;
		vih = reinterpret_cast<const VIDEOINFOHEADER*>(mt.pbFormat);
		return &vih->bmiHeader;

	} else if (mt.formattype == FORMAT_VideoInfo2) {
		const VIDEOINFOHEADER2 *vih;
		vih = reinterpret_cast<const VIDEOINFOHEADER2*>(mt.pbFormat);
		return &vih->bmiHeader;
	}

	return NULL;
}

}; /* namespace DShow */
