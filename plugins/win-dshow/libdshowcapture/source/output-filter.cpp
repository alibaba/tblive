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

#include "output-filter.hpp"
#include "log.hpp"

namespace DShow {

#if 0
#define PrintFunc(x) Debug(x)
#else
#define PrintFunc(x)
#endif

#define FILTER_NAME    L"Output Filter"
#define VIDEO_PIN_NAME L"Video Output"
#define AUDIO_PIN_NAME L"Audio Output"

OutputPin::OutputPin(OutputFilter *filter_, const PinOutputInfo &info)
	: refCount   (0),
	  outputInfo (info),
	  filter     (filter_)
{
}

OutputPin::~OutputPin()
{
}

STDMETHODIMP OutputPin::QueryInterface(REFIID riid, void **ppv)
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

STDMETHODIMP_(ULONG) OutputPin::AddRef()
{
	return (ULONG)InterlockedIncrement(&refCount);
}

STDMETHODIMP_(ULONG) OutputPin::Release()
{
	if (!InterlockedDecrement(&refCount)) {
		delete this;
		return 0;
	}

	return (ULONG)refCount;
}

// IPin methods
STDMETHODIMP OutputPin::Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt)
{
	HRESULT hr;

	PrintFunc(L"OutputPin::Connect");

	if (filter->state == State_Running)
		return VFW_E_NOT_STOPPED;

	if (connectedPin)
		return VFW_E_ALREADY_CONNECTED;

	hr = pReceivePin->ReceiveConnection(this, outputInfo.mt);
	if (FAILED(hr)) {
#if 0 /* debug code to test caps on fail */
		ComPtr<IEnumMediaTypes> enumMT;
		pReceivePin->EnumMediaTypes(&enumMT);

		if (enumMT) {
			MediaTypePtr mt;
			ULONG count = 0;

			while (enumMT->Next(1, &mt, &count) == S_OK) {
				int test = 0;
				test = 1;
			}
		}
#endif
		return E_FAIL;
	}

	ComQIPtr<IMemInputPin> memInput(pReceivePin);
	if (!memInput)
		return E_FAIL;

	if (!!allocator) {
		allocator->Decommit();
	}

	hr = memInput->GetAllocator(&allocator);
	if (hr == VFW_E_NO_ALLOCATOR)
		hr = CoCreateInstance(CLSID_MemoryAllocator, NULL,
				CLSCTX_INPROC_SERVER, __uuidof(IMemAllocator),
				(void**)&allocator);

	if (FAILED(hr))
		return E_FAIL;

	int cx = outputInfo.cx;
	int cy = outputInfo.cy;

	bufSize = cx * cy + cx * cy / 2;

	ALLOCATOR_PROPERTIES props;

	hr = memInput->GetAllocatorRequirements(&props);
	if (hr == E_NOTIMPL) {
		props.cBuffers = 4;
		props.cbBuffer = (long)bufSize;
		props.cbAlign  = 32;
		props.cbPrefix = 0;

	} else if (FAILED(hr)) {
		return E_FAIL;
	}

	ALLOCATOR_PROPERTIES actual;
	hr = allocator->SetProperties(&props, &actual);
	if (FAILED(hr))
		return E_FAIL;

	if (FAILED(allocator->Commit()))
		return E_FAIL;

	memInput->NotifyAllocator(allocator, false);

	connectedPin = pReceivePin;
	DSHOW_UNUSED(pmt);
	return S_OK;
}

STDMETHODIMP OutputPin::ReceiveConnection(IPin *pConnector,
		const AM_MEDIA_TYPE *pmt)
{
	PrintFunc(L"OutputPin::ReceiveConnection");

	DSHOW_UNUSED(pConnector);
	DSHOW_UNUSED(pmt);
	return S_OK;
}

STDMETHODIMP OutputPin::Disconnect()
{
	PrintFunc(L"OutputPin::Disconnect");

	if (!connectedPin)
		return S_FALSE;

	if (!!allocator) {
		allocator->Decommit();
	}

	connectedPin = nullptr;
	return S_OK;
}

STDMETHODIMP OutputPin::ConnectedTo(IPin **pPin)
{
	PrintFunc(L"OutputPin::ConnectedTo");

	if (!connectedPin)
		return VFW_E_NOT_CONNECTED;

	IPin *pin = connectedPin;
	pin->AddRef();
	*pPin = pin;
	return S_OK;
}

STDMETHODIMP OutputPin::ConnectionMediaType(AM_MEDIA_TYPE *pmt)
{
	PrintFunc(L"OutputPin::ConnectionMediaType");

	if (!connectedPin)
		return VFW_E_NOT_CONNECTED;

	return CopyMediaType(pmt, outputInfo.mt);
}

STDMETHODIMP OutputPin::QueryPinInfo(PIN_INFO *pInfo)
{
	PrintFunc(L"OutputPin::QueryPinInfo");

	pInfo->pFilter = filter;
	if (filter) {
		IBaseFilter *ptr = filter;
		ptr->AddRef();
	}

	if (outputInfo.expectedMajorType == MEDIATYPE_Video)
		memcpy(pInfo->achName, VIDEO_PIN_NAME, sizeof(VIDEO_PIN_NAME));
	else
		memcpy(pInfo->achName, AUDIO_PIN_NAME, sizeof(AUDIO_PIN_NAME));

	pInfo->dir = PINDIR_OUTPUT;

	return NOERROR;
}

STDMETHODIMP OutputPin::QueryDirection(PIN_DIRECTION *pPinDir)
{
	*pPinDir = PINDIR_OUTPUT;
	return NOERROR;
}

#define OUTPUT_PIN_NAME L"Output Pin"

STDMETHODIMP OutputPin::QueryId(LPWSTR *lpId)
{
	wchar_t *str = (wchar_t*)CoTaskMemAlloc(sizeof(OUTPUT_PIN_NAME));
	memcpy(str, OUTPUT_PIN_NAME, sizeof(OUTPUT_PIN_NAME));
	*lpId = str;
	return S_OK;
}

STDMETHODIMP OutputPin::QueryAccept(const AM_MEDIA_TYPE *pmt)
{
	PrintFunc(L"OutputPin::QueryAccept");

	DSHOW_UNUSED(pmt);
	return S_OK;
}

STDMETHODIMP OutputPin::EnumMediaTypes(IEnumMediaTypes **ppEnum)
{
	PrintFunc(L"OutputPin::EnumMediaTypes");

	*ppEnum = new OutputEnumMediaTypes(this);
	if (!*ppEnum)
		return E_OUTOFMEMORY;

	return NOERROR;
}

STDMETHODIMP OutputPin::QueryInternalConnections(IPin **apPin, ULONG *nPin)
{
	PrintFunc(L"OutputPin::QueryInternalConnections");

	DSHOW_UNUSED(apPin);
	DSHOW_UNUSED(nPin);
	return E_NOTIMPL;
}

STDMETHODIMP OutputPin::EndOfStream()
{
	PrintFunc(L"OutputPin::EndOfStream");

	return S_OK;
}

STDMETHODIMP OutputPin::BeginFlush()
{
	PrintFunc(L"OutputPin::BeginFlush");

	flushing = true;
	return S_OK;
}

STDMETHODIMP OutputPin::EndFlush()
{
	PrintFunc(L"OutputPin::EndFlush");

	flushing = false;
	return S_OK;
}

STDMETHODIMP OutputPin::NewSegment(REFERENCE_TIME tStart,
		REFERENCE_TIME tStop, double dRate)
{
	PrintFunc(L"OutputPin::NewSegment");

	DSHOW_UNUSED(tStart);
	DSHOW_UNUSED(tStop);
	DSHOW_UNUSED(dRate);
	return S_OK;
}

bool OutputPin::IsValidMediaType(const AM_MEDIA_TYPE *pmt) const
{
	if (pmt->pbFormat) {
		if (pmt->subtype   != outputInfo.expectedSubType ||
		    pmt->majortype != outputInfo.expectedMajorType)
			return false;

		if (outputInfo.expectedMajorType == MEDIATYPE_Video) {
			const BITMAPINFOHEADER *bih = GetBitmapInfoHeader(*pmt);
			if (!bih || bih->biHeight == 0 || bih->biWidth == 0)
				return false;
		}
	}

	return true;
}

void OutputPin::Send(unsigned char *data[DSHOW_MAX_PLANES],
		size_t linesize[DSHOW_MAX_PLANES],
		long long timestampStart, long long timestampEnd)
{
	ComQIPtr<IMemInputPin> memInput(connectedPin);
	REFERENCE_TIME startTime = timestampStart;
	REFERENCE_TIME endTime = timestampEnd;
	ComPtr<IMediaSample> sample;
	HRESULT hr;
	BYTE *ptr;

	if (!memInput || !allocator)
		return;

	hr = allocator->GetBuffer(&sample, &startTime, &endTime, 0);
	if (FAILED(hr))
		return;

	if (FAILED(sample->SetActualDataLength((long)bufSize)))
		return;
	if (FAILED(sample->SetDiscontinuity(false)))
		return;
	if (FAILED(sample->SetMediaTime(&startTime, &endTime)))
		return;
	if (FAILED(sample->SetPreroll(false)))
		return;
	if (FAILED(sample->SetTime(&startTime, &endTime)))
		return;
	if (FAILED(sample->GetPointer(&ptr)))
		return;

	size_t total = 0;
	for (size_t i = 0; i < DSHOW_MAX_PLANES; i++) {
		if (!linesize[i])
			break;

		memcpy(ptr + total, data[i], linesize[i]);
		total += linesize[i];
	}

	hr = memInput->Receive(sample);
	if (FAILED(hr)) {
		DebugHR(L"test", hr);
	}
}

void OutputPin::Stop()
{
	if (!!connectedPin) {
		connectedPin->BeginFlush();
		connectedPin->EndFlush();
	}
}

// ============================================================================

class SourceMiscFlags : public IAMFilterMiscFlags {
	volatile long refCount = 0;

public:
	inline SourceMiscFlags() {}
	virtual ~SourceMiscFlags() {}

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
		return AM_FILTER_MISC_FLAGS_IS_SOURCE;
	}
};

OutputFilter::OutputFilter(const PinOutputInfo &info)
	: refCount (0),
	  state    (State_Stopped),
	  pin      (new OutputPin(this, info)),
	  misc     (new SourceMiscFlags)
{
}

OutputFilter::~OutputFilter()
{
}

// IUnknown methods
STDMETHODIMP OutputFilter::QueryInterface(REFIID riid, void **ppv)
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

STDMETHODIMP_(ULONG) OutputFilter::AddRef()
{
	return InterlockedIncrement(&refCount);
}

STDMETHODIMP_(ULONG) OutputFilter::Release()
{
	if (!InterlockedDecrement(&refCount)) {
		delete this;
		return 0;
	}

	return refCount;
}

// IPersist method
STDMETHODIMP OutputFilter::GetClassID(CLSID *pClsID)
{
	DSHOW_UNUSED(pClsID);
	return E_NOTIMPL;
}

// IMediaFilter methods
STDMETHODIMP OutputFilter::GetState(DWORD dwMSecs, FILTER_STATE *State)
{
	PrintFunc(L"OutputFilter::GetState");

	*State = state;

	DSHOW_UNUSED(dwMSecs);
	return S_OK;
}

STDMETHODIMP OutputFilter::SetSyncSource(IReferenceClock *pClock)
{
	DSHOW_UNUSED(pClock);
	return S_OK;
}

STDMETHODIMP OutputFilter::GetSyncSource(IReferenceClock **pClock)
{
	*pClock = nullptr;
	return NOERROR;
}

STDMETHODIMP OutputFilter::Stop()
{
	PrintFunc(L"OutputFilter::Stop");

	if (state != State_Stopped) {
		pin->Stop();
	}

	state = State_Stopped;
	return S_OK;
}

STDMETHODIMP OutputFilter::Pause()
{
	PrintFunc(L"OutputFilter::Pause");

	state = State_Paused;
	return S_OK;
}

STDMETHODIMP OutputFilter::Run(REFERENCE_TIME tStart)
{
	PrintFunc(L"OutputFilter::Run");

	state = State_Running;

	DSHOW_UNUSED(tStart);
	return S_OK;
}

// IBaseFilter methods
STDMETHODIMP OutputFilter::EnumPins(IEnumPins **ppEnum)
{
	PrintFunc(L"OutputFilter::EnumPins");

	*ppEnum = new OutputEnumPins(this, nullptr);
	return (*ppEnum == nullptr) ? E_OUTOFMEMORY : NOERROR;
}

STDMETHODIMP OutputFilter::FindPin(LPCWSTR Id, IPin **ppPin)
{
	PrintFunc(L"OutputFilter::FindPin");

	DSHOW_UNUSED(Id);
	DSHOW_UNUSED(ppPin);
	return E_NOTIMPL;
}

STDMETHODIMP OutputFilter::QueryFilterInfo(FILTER_INFO *pInfo)
{
	PrintFunc(L"OutputFilter::QueryFilterInfo");

	memcpy(pInfo->achName, FILTER_NAME, sizeof(FILTER_NAME));

	pInfo->pGraph = graph;
	if (graph) {
		IFilterGraph *graph_ptr = graph;
		graph_ptr->AddRef();
	}
	return NOERROR;
}

STDMETHODIMP OutputFilter::JoinFilterGraph(IFilterGraph *pGraph, LPCWSTR pName)
{
	DSHOW_UNUSED(pName);

	graph = pGraph;
	return NOERROR;
}

STDMETHODIMP OutputFilter::QueryVendorInfo(LPWSTR *pVendorInfo)
{
	DSHOW_UNUSED(pVendorInfo);
	return E_NOTIMPL;
}

// ============================================================================

OutputEnumPins::OutputEnumPins(OutputFilter *filter_, OutputEnumPins *pEnum)
	: filter (filter_)
{
	curPin = (pEnum != nullptr) ? pEnum->curPin : 0;
}

OutputEnumPins::~OutputEnumPins()
{
}

// IUnknown
STDMETHODIMP OutputEnumPins::QueryInterface(REFIID riid, void **ppv)
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

STDMETHODIMP_(ULONG) OutputEnumPins::AddRef()
{
	return (ULONG)InterlockedIncrement(&refCount);
}

STDMETHODIMP_(ULONG) OutputEnumPins::Release()
{
	if (!InterlockedDecrement(&refCount)) {
		delete this;
		return 0;
	}

	return (ULONG)refCount;
}

// IEnumPins
STDMETHODIMP OutputEnumPins::Next(ULONG cPins, IPin **ppPins, ULONG *pcFetched)
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

STDMETHODIMP OutputEnumPins::Skip(ULONG cPins)
{
	return ((curPin += cPins) > 1) ? S_FALSE : S_OK;
}

STDMETHODIMP OutputEnumPins::Reset()
{
	curPin = 0;
	return S_OK;
}

STDMETHODIMP OutputEnumPins::Clone(IEnumPins **ppEnum)
{
	*ppEnum = new OutputEnumPins(filter, this);
	return (*ppEnum == nullptr) ? E_OUTOFMEMORY : NOERROR;
}


// ============================================================================

OutputEnumMediaTypes::OutputEnumMediaTypes(OutputPin *pin_)
	: pin (pin_)
{
}

OutputEnumMediaTypes::~OutputEnumMediaTypes()
{
}

STDMETHODIMP OutputEnumMediaTypes::QueryInterface(REFIID riid, void **ppv)
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

STDMETHODIMP_(ULONG) OutputEnumMediaTypes::AddRef()
{
	return (ULONG)InterlockedIncrement(&refCount);
}

STDMETHODIMP_(ULONG) OutputEnumMediaTypes::Release()
{
	if (!InterlockedDecrement(&refCount)) {
		delete this;
		return 0;
	}

	return (ULONG)refCount;
}

// IEnumMediaTypes
STDMETHODIMP OutputEnumMediaTypes::Next(ULONG cMediaTypes,
		AM_MEDIA_TYPE **ppMediaTypes, ULONG *pcFetched)
{
	PrintFunc(L"OutputEnumMediaTypes::Next");

	UINT nFetched = 0;

	if (curMT == 0 && cMediaTypes > 0) {
		*ppMediaTypes = pin->outputInfo.mt.Duplicate();
		nFetched = 1;
		curMT++;
	}

	if (pcFetched) *pcFetched = nFetched;

	return (nFetched == cMediaTypes) ? S_OK : S_FALSE;
}

STDMETHODIMP OutputEnumMediaTypes::Skip(ULONG cMediaTypes)
{
	PrintFunc(L"OutputEnumMediaTypes::Skip");

	return ((curMT += cMediaTypes) > 1) ? S_FALSE : S_OK;
}

STDMETHODIMP OutputEnumMediaTypes::Reset()
{
	PrintFunc(L"OutputEnumMediaTypes::Reset");

	curMT = 0;
	return S_OK;
}

STDMETHODIMP OutputEnumMediaTypes::Clone(IEnumMediaTypes **ppEnum)
{
	*ppEnum = new OutputEnumMediaTypes(pin);
	return (*ppEnum == nullptr) ? E_OUTOFMEMORY : NOERROR;
}

}; /* namespace DShow */
