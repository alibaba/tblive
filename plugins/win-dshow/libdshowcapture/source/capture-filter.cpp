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

#include "capture-filter.hpp"
#include "log.hpp"

namespace DShow {

#if 0
#define PrintFunc(x) Debug(x)
#else
#define PrintFunc(x)
#endif

#define FILTER_NAME    L"Capture Filter"
#define VIDEO_PIN_NAME L"Video Capture"
#define AUDIO_PIN_NAME L"Audio Capture"

CapturePin::CapturePin(CaptureFilter *filter_, const PinCaptureInfo &info)
	: refCount    (0),
	  captureInfo (info),
	  filter      (filter_)
{
	connectedMediaType->majortype = info.expectedMajorType;
}

CapturePin::~CapturePin()
{
}

STDMETHODIMP CapturePin::QueryInterface(REFIID riid, void **ppv)
{
	if (riid == IID_IUnknown) {
		AddRef();
		*ppv = this;
	} else if (riid == IID_IPin) {
		AddRef();
		*ppv = (IPin*)this;
	} else if (riid == IID_IMemInputPin) {
		AddRef();
		*ppv = (IMemInputPin*)this;
	} else {
		*ppv = nullptr;
		return E_NOINTERFACE;
	}

	return NOERROR;
}

STDMETHODIMP_(ULONG) CapturePin::AddRef()
{
	return (ULONG)InterlockedIncrement(&refCount);
}

STDMETHODIMP_(ULONG) CapturePin::Release()
{
	if (!InterlockedDecrement(&refCount)) {
		delete this;
		return 0;
	}

	return (ULONG)refCount;
}

// IPin methods
STDMETHODIMP CapturePin::Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt)
{
	PrintFunc(L"CapturePin::Connect");

	if (filter->state == State_Running)
		return VFW_E_NOT_STOPPED;

	if (connectedPin)
		return VFW_E_ALREADY_CONNECTED;

	if (!pmt)
		return S_OK;

	if (pmt->majortype != GUID_NULL &&
	    pmt->majortype != captureInfo.expectedMajorType)
		return S_FALSE;

	if (pmt->majortype == captureInfo.expectedMajorType &&
	    !IsValidMediaType(pmt))
		return S_FALSE;

	DSHOW_UNUSED(pReceivePin);
	return S_OK;
}

STDMETHODIMP CapturePin::ReceiveConnection(IPin *pConnector,
		const AM_MEDIA_TYPE *pmt)
{
	PrintFunc(L"CapturePin::ReceiveConnection");

	if (filter->state != State_Stopped)
		return VFW_E_NOT_STOPPED;
	if (!pConnector || !pmt)
		return E_POINTER;
	if (connectedPin)
		return VFW_E_ALREADY_CONNECTED;

	if (QueryAccept(pmt) != S_OK)
		return VFW_E_TYPE_NOT_ACCEPTED;

	connectedPin       = pConnector;
	connectedMediaType = pmt;

	return S_OK;
}

STDMETHODIMP CapturePin::Disconnect()
{
	PrintFunc(L"CapturePin::Disconnect");

	if (!connectedPin)
		return S_FALSE;

	connectedPin = nullptr;
	return S_OK;
}


STDMETHODIMP CapturePin::ConnectedTo(IPin **pPin)
{
	PrintFunc(L"CapturePin::ConnectedTo");

	if (!connectedPin)
		return VFW_E_NOT_CONNECTED;

	IPin *pin = connectedPin;
	pin->AddRef();
	*pPin = pin;
	return S_OK;
}

STDMETHODIMP CapturePin::ConnectionMediaType(AM_MEDIA_TYPE *pmt)
{
	PrintFunc(L"CapturePin::ConnectionMediaType");

	if (!connectedPin)
		return VFW_E_NOT_CONNECTED;

	return CopyMediaType(pmt, connectedMediaType);
}

STDMETHODIMP CapturePin::QueryPinInfo(PIN_INFO *pInfo)
{
	PrintFunc(L"CapturePin::QueryPinInfo");

	pInfo->pFilter = filter;
	if (filter) {
		IBaseFilter *ptr = filter;
		ptr->AddRef();
	}

	if (captureInfo.expectedMajorType == MEDIATYPE_Video)
		memcpy(pInfo->achName, VIDEO_PIN_NAME, sizeof(VIDEO_PIN_NAME));
	else
		memcpy(pInfo->achName, AUDIO_PIN_NAME, sizeof(AUDIO_PIN_NAME));

	pInfo->dir = PINDIR_INPUT;

	return NOERROR;
}

STDMETHODIMP CapturePin::QueryDirection(PIN_DIRECTION *pPinDir)
{
	*pPinDir = PINDIR_INPUT;
	return NOERROR;
}

#define CAPTURE_PIN_NAME L"Capture Pin"

STDMETHODIMP CapturePin::QueryId(LPWSTR *lpId)
{
	wchar_t *str = (wchar_t*)CoTaskMemAlloc(sizeof(CAPTURE_PIN_NAME));
	memcpy(str, CAPTURE_PIN_NAME, sizeof(CAPTURE_PIN_NAME));
	*lpId = str;
	return S_OK;
}

STDMETHODIMP CapturePin::QueryAccept(const AM_MEDIA_TYPE *pmt)
{
	PrintFunc(L"CapturePin::QueryAccept");

	if (pmt->majortype != captureInfo.expectedMajorType)
		return S_FALSE;
	if (!IsValidMediaType(pmt))
		return S_FALSE;

	if (connectedPin)
		connectedMediaType = pmt;

	return S_OK;
}

STDMETHODIMP CapturePin::EnumMediaTypes(IEnumMediaTypes **ppEnum)
{
	PrintFunc(L"CapturePin::EnumMediaTypes");

	*ppEnum = new CaptureEnumMediaTypes(this);
	if (!*ppEnum)
		return E_OUTOFMEMORY;

	return NOERROR;
}

STDMETHODIMP CapturePin::QueryInternalConnections(IPin **apPin, ULONG *nPin)
{
	PrintFunc(L"CapturePin::QueryInternalConnections");

	DSHOW_UNUSED(apPin);
	DSHOW_UNUSED(nPin);
	return E_NOTIMPL;
}

STDMETHODIMP CapturePin::EndOfStream()
{
	PrintFunc(L"CapturePin::EndOfStream");

	return S_OK;
}

STDMETHODIMP CapturePin::BeginFlush()
{
	PrintFunc(L"CapturePin::BeginFlush");

	flushing = true;
	return S_OK;
}

STDMETHODIMP CapturePin::EndFlush()
{
	PrintFunc(L"CapturePin::EndFlush");

	flushing = false;
	return S_OK;
}

STDMETHODIMP CapturePin::NewSegment(REFERENCE_TIME tStart,
		REFERENCE_TIME tStop, double dRate)
{
	PrintFunc(L"CapturePin::NewSegment");

	DSHOW_UNUSED(tStart);
	DSHOW_UNUSED(tStop);
	DSHOW_UNUSED(dRate);
	return S_OK;
}

// IMemInputPin methods
STDMETHODIMP CapturePin::GetAllocator(IMemAllocator **ppAllocator)
{
	PrintFunc(L"CapturePin::GetAllocator");

	DSHOW_UNUSED(ppAllocator);
	return VFW_E_NO_ALLOCATOR;
}

STDMETHODIMP CapturePin::NotifyAllocator(IMemAllocator *pAllocator,
		BOOL bReadOnly)
{
	PrintFunc(L"CapturePin::NotifyAllocator");

	DSHOW_UNUSED(pAllocator);
	DSHOW_UNUSED(bReadOnly);
	return S_OK;
}

STDMETHODIMP CapturePin::GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps)
{
	PrintFunc(L"CapturePin::GetAllocatorRequirements");

	DSHOW_UNUSED(pProps);
	return E_NOTIMPL;
}

STDMETHODIMP CapturePin::Receive(IMediaSample *pSample)
{
	PrintFunc(L"CapturePin::Receive");

	if (flushing)
		return S_FALSE;

	if (pSample)
		captureInfo.callback(pSample);

	return S_OK;
}

STDMETHODIMP CapturePin::ReceiveMultiple(IMediaSample **pSamples,
		long nSamples, long *nSamplesProcessed)
{
	PrintFunc(L"CapturePin::ReceiveMultiple");

	if (flushing)
		return S_FALSE;

	for (long i = 0; i < nSamples; i++)
		if (pSamples[i])
			captureInfo.callback(pSamples[i]);

	*nSamplesProcessed = nSamples;

	return S_OK;
}

STDMETHODIMP CapturePin::ReceiveCanBlock() {return S_FALSE;}

bool CapturePin::IsValidMediaType(const AM_MEDIA_TYPE *pmt) const
{
	if (pmt->pbFormat) {
		if (pmt->subtype   != captureInfo.expectedSubType ||
		    pmt->majortype != captureInfo.expectedMajorType)
			return false;

		if (captureInfo.expectedMajorType == MEDIATYPE_Video) {
			const BITMAPINFOHEADER *bih = GetBitmapInfoHeader(*pmt);
			if (!bih || bih->biHeight == 0 || bih->biWidth == 0)
				return false;
		}
	}

	return true;
}

// ============================================================================

class MiscFlagsHandler : public IAMFilterMiscFlags {
	volatile long refCount = 0;

public:
	inline MiscFlagsHandler() {}
	virtual ~MiscFlagsHandler() {}

	STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
	{
		if (riid == IID_IUnknown) {
			AddRef();
			*ppv = this;
		} else {
			*ppv = nullptr;
			return E_NOINTERFACE;
		}

		return NOERROR;
	}

	STDMETHODIMP_(ULONG) AddRef()
	{
		return InterlockedIncrement(&refCount);
	}

	STDMETHODIMP_(ULONG) Release()
	{
		if (!InterlockedDecrement(&refCount)) {
			delete this;
			return 0;
		}

		return refCount;
	}

	STDMETHODIMP_(ULONG) GetMiscFlags()
	{
		return AM_FILTER_MISC_FLAGS_IS_RENDERER;
	}
};

CaptureFilter::CaptureFilter(const PinCaptureInfo &info)
	: refCount (0),
	  state    (State_Stopped),
	  pin      (new CapturePin(this, info)),
	  misc     (new MiscFlagsHandler)
{
}

CaptureFilter::~CaptureFilter()
{
}

// IUnknown methods
STDMETHODIMP CaptureFilter::QueryInterface(REFIID riid, void **ppv)
{
	if (riid == IID_IUnknown) {
		AddRef();
		*ppv = this;
	} else if (riid == IID_IPersist) {
		AddRef();
		*ppv = (IPersist*)this;
	} else if (riid == IID_IMediaFilter) {
		AddRef();
		*ppv = (IMediaFilter*)this;
	} else if (riid == IID_IBaseFilter) {
		AddRef();
		*ppv = (IBaseFilter*)this;
	} else if (riid == IID_IAMFilterMiscFlags) {
		misc.CopyTo((IAMFilterMiscFlags**)ppv);
	} else {
		*ppv = nullptr;
		return E_NOINTERFACE;
	}

	return NOERROR;
}

STDMETHODIMP_(ULONG) CaptureFilter::AddRef()
{
	return InterlockedIncrement(&refCount);
}

STDMETHODIMP_(ULONG) CaptureFilter::Release()
{
	if (!InterlockedDecrement(&refCount)) {
		delete this;
		return 0;
	}

	return refCount;
}

// IPersist method
STDMETHODIMP CaptureFilter::GetClassID(CLSID *pClsID)
{
	DSHOW_UNUSED(pClsID);
	return E_NOTIMPL;
}

// IMediaFilter methods
STDMETHODIMP CaptureFilter::GetState(DWORD dwMSecs, FILTER_STATE *State)
{
	PrintFunc(L"CaptureFilter::GetState");

	*State = state;

	DSHOW_UNUSED(dwMSecs);
	return S_OK;
}

STDMETHODIMP CaptureFilter::SetSyncSource(IReferenceClock *pClock)
{
	DSHOW_UNUSED(pClock);
	return S_OK;
}

STDMETHODIMP CaptureFilter::GetSyncSource(IReferenceClock **pClock)
{
	*pClock = nullptr;
	return NOERROR;
}

STDMETHODIMP CaptureFilter::Stop()
{
	PrintFunc(L"CaptureFilter::Stop");

	state = State_Stopped;
	return S_OK;
}

STDMETHODIMP CaptureFilter::Pause()
{
	PrintFunc(L"CaptureFilter::Pause");

	state = State_Paused;
	return S_OK;
}

STDMETHODIMP CaptureFilter::Run(REFERENCE_TIME tStart)
{
	PrintFunc(L"CaptureFilter::Run");

	state = State_Running;

	DSHOW_UNUSED(tStart);
	return S_OK;
}

// IBaseFilter methods
STDMETHODIMP CaptureFilter::EnumPins(IEnumPins **ppEnum)
{
	PrintFunc(L"CaptureFilter::EnumPins");

	*ppEnum = new CaptureEnumPins(this, nullptr);
	return (*ppEnum == nullptr) ? E_OUTOFMEMORY : NOERROR;
}

STDMETHODIMP CaptureFilter::FindPin(LPCWSTR Id, IPin **ppPin)
{
	PrintFunc(L"CaptureFilter::FindPin");

	DSHOW_UNUSED(Id);
	DSHOW_UNUSED(ppPin);
	return E_NOTIMPL;
}

STDMETHODIMP CaptureFilter::QueryFilterInfo(FILTER_INFO *pInfo)
{
	PrintFunc(L"CaptureFilter::QueryFilterInfo");

	memcpy(pInfo->achName, FILTER_NAME, sizeof(FILTER_NAME));

	pInfo->pGraph = graph;
	if (graph) {
		IFilterGraph *graph_ptr = graph;
		graph_ptr->AddRef();
	}
	return NOERROR;
}

STDMETHODIMP CaptureFilter::JoinFilterGraph(IFilterGraph *pGraph, LPCWSTR pName)
{
	DSHOW_UNUSED(pName);

	graph = pGraph;
	return NOERROR;
}

STDMETHODIMP CaptureFilter::QueryVendorInfo(LPWSTR *pVendorInfo)
{
	DSHOW_UNUSED(pVendorInfo);
	return E_NOTIMPL;
}

// ============================================================================

CaptureEnumPins::CaptureEnumPins(CaptureFilter *filter_, CaptureEnumPins *pEnum)
	: filter (filter_)
{
	curPin = (pEnum != nullptr) ? pEnum->curPin : 0;
}

CaptureEnumPins::~CaptureEnumPins()
{
}

// IUnknown
STDMETHODIMP CaptureEnumPins::QueryInterface(REFIID riid, void **ppv)
{
	if (riid == IID_IUnknown || riid == IID_IEnumPins) {
		AddRef();
		*ppv = (IEnumPins*)this;
		return NOERROR;
	} else {
		*ppv = nullptr;
		return E_NOINTERFACE;
	}
}

STDMETHODIMP_(ULONG) CaptureEnumPins::AddRef()
{
	return (ULONG)InterlockedIncrement(&refCount);
}

STDMETHODIMP_(ULONG) CaptureEnumPins::Release()
{
	if (!InterlockedDecrement(&refCount)) {
		delete this;
		return 0;
	}

	return (ULONG)refCount;
}

// IEnumPins
STDMETHODIMP CaptureEnumPins::Next(ULONG cPins, IPin **ppPins, ULONG *pcFetched)
{
	UINT nFetched = 0;

	if (curPin == 0 && cPins > 0) {
		IPin *pPin = filter->GetPin();

		*ppPins = pPin;
		pPin->AddRef();

		nFetched = 1;
		curPin++;
	}

	if (pcFetched) *pcFetched = nFetched;

	return (nFetched == cPins) ? S_OK : S_FALSE;
}

STDMETHODIMP CaptureEnumPins::Skip(ULONG cPins)
{
	return ((curPin += cPins) > 1) ? S_FALSE : S_OK;
}

STDMETHODIMP CaptureEnumPins::Reset()
{
	curPin = 0;
	return S_OK;
}

STDMETHODIMP CaptureEnumPins::Clone(IEnumPins **ppEnum)
{
	*ppEnum = new CaptureEnumPins(filter, this);
	return (*ppEnum == nullptr) ? E_OUTOFMEMORY : NOERROR;
}


// ============================================================================

CaptureEnumMediaTypes::CaptureEnumMediaTypes(CapturePin *pin_)
	: pin (pin_)
{
}

CaptureEnumMediaTypes::~CaptureEnumMediaTypes()
{
}

STDMETHODIMP CaptureEnumMediaTypes::QueryInterface(REFIID riid, void **ppv)
{
	if (riid == IID_IUnknown || riid == IID_IEnumMediaTypes) {
		AddRef();
		*ppv = this;
		return NOERROR;
	} else {
		*ppv = nullptr;
		return E_NOINTERFACE;
	}
}

STDMETHODIMP_(ULONG) CaptureEnumMediaTypes::AddRef()
{
	return (ULONG)InterlockedIncrement(&refCount);
}

STDMETHODIMP_(ULONG) CaptureEnumMediaTypes::Release()
{
	if (!InterlockedDecrement(&refCount)) {
		delete this;
		return 0;
	}

	return (ULONG)refCount;
}

// IEnumMediaTypes
STDMETHODIMP CaptureEnumMediaTypes::Next(ULONG cMediaTypes,
		AM_MEDIA_TYPE **ppMediaTypes, ULONG *pcFetched)
{
	PrintFunc(L"CaptureEnumMediaTypes::Next");

	UINT nFetched = 0;

	if (curMT == 0 && cMediaTypes > 0) {
		*ppMediaTypes = pin->connectedMediaType.Duplicate();
		nFetched = 1;
		curMT++;
	}

	if (pcFetched) *pcFetched = nFetched;

	return (nFetched == cMediaTypes) ? S_OK : S_FALSE;
}

STDMETHODIMP CaptureEnumMediaTypes::Skip(ULONG cMediaTypes)
{
	PrintFunc(L"CaptureEnumMediaTypes::Skip");

	return ((curMT += cMediaTypes) > 1) ? S_FALSE : S_OK;
}

STDMETHODIMP CaptureEnumMediaTypes::Reset()
{
	PrintFunc(L"CaptureEnumMediaTypes::Reset");

	curMT = 0;
	return S_OK;
}

STDMETHODIMP CaptureEnumMediaTypes::Clone(IEnumMediaTypes **ppEnum)
{
	*ppEnum = new CaptureEnumMediaTypes(pin);
	return (*ppEnum == nullptr) ? E_OUTOFMEMORY : NOERROR;
}

}; /* namespace DShow */
