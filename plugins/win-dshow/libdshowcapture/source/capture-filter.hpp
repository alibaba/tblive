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
#include "dshow-media-type.hpp"
#include "../dshowcapture.hpp"

namespace DShow {

class CaptureFilter;
class CaptureSource;

typedef void (*CaptureCallback)(void *param, IMediaSample *sample);

struct PinCaptureInfo {
	std::function<void (IMediaSample *sample)> callback;
	GUID                                       expectedMajorType;
	GUID                                       expectedSubType;
};

class CapturePin : public IPin, public IMemInputPin {
	friend class CaptureEnumMediaTypes;

	volatile long          refCount;

	PinCaptureInfo         captureInfo;
	ComPtr<IPin>           connectedPin;
	CaptureFilter          *filter;
	MediaType              connectedMediaType;
	volatile bool          flushing = false;

	bool IsValidMediaType(const AM_MEDIA_TYPE *pmt) const;

public:
	CapturePin(CaptureFilter *filter, const PinCaptureInfo &info);
	virtual ~CapturePin();

	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IPin methods
	STDMETHODIMP Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt);
	STDMETHODIMP ReceiveConnection(IPin *connector,
			const AM_MEDIA_TYPE *pmt);
	STDMETHODIMP Disconnect();
	STDMETHODIMP ConnectedTo(IPin **pPin);
	STDMETHODIMP ConnectionMediaType(AM_MEDIA_TYPE *pmt);
	STDMETHODIMP QueryPinInfo(PIN_INFO *pInfo);
	STDMETHODIMP QueryDirection(PIN_DIRECTION *pPinDir);
	STDMETHODIMP QueryId(LPWSTR *lpId);
	STDMETHODIMP QueryAccept(const AM_MEDIA_TYPE *pmt);
	STDMETHODIMP EnumMediaTypes(IEnumMediaTypes **ppEnum);
	STDMETHODIMP QueryInternalConnections(IPin* *apPin, ULONG *nPin);
	STDMETHODIMP EndOfStream();

	STDMETHODIMP BeginFlush();
	STDMETHODIMP EndFlush();
	STDMETHODIMP NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop,
			double dRate);

	// IMemInputPin methods
	STDMETHODIMP GetAllocator(IMemAllocator **ppAllocator);
	STDMETHODIMP NotifyAllocator(IMemAllocator *pAllocator, BOOL bReadOnly);
	STDMETHODIMP GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps);
	STDMETHODIMP Receive(IMediaSample *pSample);
	STDMETHODIMP ReceiveMultiple(IMediaSample **pSamples, long nSamples,
			long *nSamplesProcessed);
	STDMETHODIMP ReceiveCanBlock();
};

class CaptureFilter : public IBaseFilter {
	friend class CapturePin;

	volatile long         refCount;
	FILTER_STATE          state;
	ComPtr<IFilterGraph>  graph;
	ComPtr<CapturePin>    pin;

	ComPtr<IAMFilterMiscFlags> misc;

public:
	CaptureFilter(const PinCaptureInfo &info);
	virtual ~CaptureFilter();

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IPersist method
	STDMETHODIMP GetClassID(CLSID *pClsID);

	// IMediaFilter methods
	STDMETHODIMP GetState(DWORD dwMSecs, FILTER_STATE *State);
	STDMETHODIMP SetSyncSource(IReferenceClock *pClock);
	STDMETHODIMP GetSyncSource(IReferenceClock **pClock);
	STDMETHODIMP Stop();
	STDMETHODIMP Pause();
	STDMETHODIMP Run(REFERENCE_TIME tStart);

	// IBaseFilter methods
	STDMETHODIMP EnumPins(IEnumPins **ppEnum);
	STDMETHODIMP FindPin(LPCWSTR Id, IPin **ppPin);
	STDMETHODIMP QueryFilterInfo(FILTER_INFO *pInfo);
	STDMETHODIMP JoinFilterGraph(IFilterGraph *pGraph, LPCWSTR pName);
	STDMETHODIMP QueryVendorInfo(LPWSTR *pVendorInfo);

	inline CapturePin* GetPin() const {return (CapturePin*)pin;}
};

class CaptureEnumPins : public IEnumPins {
	volatile long          refCount = 1;
	ComPtr<CaptureFilter>  filter;
	UINT                   curPin;

public:
	CaptureEnumPins(CaptureFilter *filter, CaptureEnumPins *pEnum);
	virtual ~CaptureEnumPins();

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IEnumPins
	STDMETHODIMP Next(ULONG cPins, IPin **ppPins, ULONG *pcFetched);
	STDMETHODIMP Skip(ULONG cPins);
	STDMETHODIMP Reset();
	STDMETHODIMP Clone(IEnumPins **ppEnum);
};

class CaptureEnumMediaTypes : public IEnumMediaTypes {
	volatile long       refCount = 1;
	ComPtr<CapturePin>  pin;
	UINT                curMT = 0;

public:
	CaptureEnumMediaTypes(CapturePin *pin);
	virtual ~CaptureEnumMediaTypes();

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IEnumMediaTypes
	STDMETHODIMP Next(ULONG cMediaTypes, AM_MEDIA_TYPE **ppMediaTypes,
			ULONG *pcFetched);
	STDMETHODIMP Skip(ULONG cMediaTypes);
	STDMETHODIMP Reset();
	STDMETHODIMP Clone(IEnumMediaTypes **ppEnum);
};

}; /* namespace DShow */
