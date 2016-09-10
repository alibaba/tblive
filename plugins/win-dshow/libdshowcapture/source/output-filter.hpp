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

struct PinOutputInfo {
	MediaType mt;
	GUID expectedMajorType;
	GUID expectedSubType;
	int cx;
	int cy;
};

class OutputFilter;

class OutputPin : public IPin {
	friend class OutputEnumMediaTypes;

	volatile long                  refCount;

	PinOutputInfo                  outputInfo;
	ComPtr<IPin>                   connectedPin;
	OutputFilter                   *filter;
	volatile bool                  flushing = false;
	ComPtr<IMemAllocator>          allocator;
	size_t                         bufSize;

	bool IsValidMediaType(const AM_MEDIA_TYPE *pmt) const;

public:
	OutputPin(OutputFilter *filter, const PinOutputInfo &info);
	virtual ~OutputPin();

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

	void Send(unsigned char *data[DSHOW_MAX_PLANES],
			size_t linesize[DSHOW_MAX_PLANES],
			long long timestampStart,
			long long timestampEnd);

	void Stop();
};

class OutputFilter : public IBaseFilter {
	friend class OutputPin;

	volatile long                  refCount;
	FILTER_STATE                   state;
	ComPtr<IFilterGraph>           graph;
	ComPtr<OutputPin>              pin;

	ComPtr<IAMFilterMiscFlags>     misc;

public:
	OutputFilter(const PinOutputInfo &info);
	virtual ~OutputFilter();

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

	inline OutputPin* GetPin() const {return (OutputPin*)pin;}

	inline void Send(unsigned char *data[DSHOW_MAX_PLANES],
			size_t linesize[DSHOW_MAX_PLANES],
			long long timestampStart, long long timestampEnd)
	{
		pin->Send(data, linesize, timestampStart, timestampEnd);
	}
};

class OutputEnumPins : public IEnumPins {
	volatile long                  refCount = 1;
	ComPtr<OutputFilter>           filter;
	UINT                           curPin;

public:
	OutputEnumPins(OutputFilter *filter, OutputEnumPins *pEnum);
	virtual ~OutputEnumPins();

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

class OutputEnumMediaTypes : public IEnumMediaTypes {
	volatile long                  refCount = 1;
	ComPtr<OutputPin>              pin;
	UINT                           curMT = 0;

public:
	OutputEnumMediaTypes(OutputPin *pin);
	virtual ~OutputEnumMediaTypes();

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

