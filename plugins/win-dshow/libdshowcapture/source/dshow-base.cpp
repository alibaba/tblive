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

#include "dshow-base.hpp"
#include "dshow-enum.hpp"
#include "log.hpp"

#include <bdaiface.h>

#include <vector>
#include <string>

using namespace std;

namespace DShow {

bool CreateFilterGraph(IGraphBuilder **pgraph, ICaptureGraphBuilder2 **pbuilder,
		IMediaControl **pcontrol)
{
	ComPtr<IGraphBuilder> graph;
	ComPtr<ICaptureGraphBuilder2> builder;
	ComPtr<IMediaControl> control;
	HRESULT hr;

	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
			IID_IFilterGraph, (void**)&graph);
	if (FAILED(hr)) {
		ErrorHR(L"Failed to create IGraphBuilder", hr);
		return false;
	}

	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL,
			CLSCTX_INPROC_SERVER,
			IID_ICaptureGraphBuilder2, (void**)&builder);
	if (FAILED(hr)) {
		ErrorHR(L"Failed to create ICaptureGraphBuilder2", hr);
		return false;
	}

	hr = builder->SetFiltergraph(graph);
	if (FAILED(hr)) {
		ErrorHR(L"Failed to set filter graph", hr);
		return false;
	}

	hr = graph->QueryInterface(IID_IMediaControl, (void**)&control);
	if (FAILED(hr)) {
		ErrorHR(L"Failed to create IMediaControl", hr);
		return false;
	}

	*pgraph = graph.Detach();
	*pbuilder = builder.Detach();
	*pcontrol = control.Detach();
	return true;
}

void LogFilters(IGraphBuilder *graph)
{
	ComPtr<IEnumFilters> filterEnum;
	ComPtr<IBaseFilter>  filter;
	HRESULT hr;

	hr = graph->EnumFilters(&filterEnum);
	if (FAILED(hr))
		return;

	Debug(L"Loaded filters:");

	while (filterEnum->Next(1, &filter, NULL) == S_OK) {
		FILTER_INFO filterInfo;

		hr = filter->QueryFilterInfo(&filterInfo);
		if (SUCCEEDED(hr)) {
			if (filterInfo.pGraph)
				filterInfo.pGraph->Release();

			Debug(L"\t%s", filterInfo.achName);
		}
	}
}

struct DeviceFilterCallbackInfo {
	ComPtr<IBaseFilter>  filter;
	const wchar_t        *name;
	const wchar_t        *path;
};

static bool GetDeviceCallback(DeviceFilterCallbackInfo &info,
		IBaseFilter *filter, const wchar_t *name, const wchar_t *path)
{
	if (info.name && *info.name && wcscmp(name, info.name) != 0)
		return true;

	info.filter = filter;

	/* continue if path does not match */
	if (!path || !info.path || wcscmp(path, info.path) != 0)
		return true;

	return false;
}

bool GetDeviceFilter(const IID &type, const wchar_t *name, const wchar_t *path,
		IBaseFilter **out)
{
	DeviceFilterCallbackInfo info;
	info.name = name;
	info.path = path;

	if (!EnumDevices(type, EnumDeviceCallback(GetDeviceCallback), &info))
		return false;

	if (info.filter != NULL) {
		*out = info.filter.Detach();
		return true;
	}

	return false;
}

/* checks to see if a pin's config caps have a specific media type */
static bool PinConfigHasMajorType(IPin *pin, const GUID &type)
{
	HRESULT hr;
	ComPtr<IAMStreamConfig> config;
	int count, size;

	hr = pin->QueryInterface(IID_IAMStreamConfig, (void**)&config);
	if (FAILED(hr))
		return false;

	hr = config->GetNumberOfCapabilities(&count, &size);
	if (FAILED(hr))
		return false;

	vector<BYTE> caps;
	caps.resize(size);

	for (int i = 0; i < count; i++) {
		MediaTypePtr mt;
		if (SUCCEEDED(config->GetStreamCaps(i, &mt, caps.data())))
			if (mt->majortype == type)
				return true;
	}

	return false;
}

/* checks to see if a pin has a certain major media type */
static bool PinHasMajorType(IPin *pin, const GUID &type)
{
	HRESULT hr;
	MediaTypePtr mt;
	ComPtr<IEnumMediaTypes> mediaEnum;

	/* first, check the config caps. */
	if (PinConfigHasMajorType(pin, type))
		return true;

	/* then let's check the media type for the pin */
	if (FAILED(pin->EnumMediaTypes(&mediaEnum)))
		return false;

	ULONG curVal;
	hr = mediaEnum->Next(1, &mt, &curVal);
	if (hr != S_OK)
		return false;

	return mt->majortype == type;
}

static inline bool PinIsDirection(IPin *pin, PIN_DIRECTION dir)
{
	if (!pin)
		return false;

	PIN_DIRECTION pinDir;
	return SUCCEEDED(pin->QueryDirection(&pinDir)) && pinDir == dir;
}

static HRESULT GetPinCategory(IPin *pin, GUID &category)
{
	if (!pin)
		return E_POINTER;

	ComQIPtr<IKsPropertySet>  propertySet(pin);
	DWORD                     size;

	if (propertySet == NULL)
		return E_NOINTERFACE;

	return propertySet->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY,
			NULL, 0, &category, sizeof(GUID), &size);
}

static inline bool PinIsCategory(IPin *pin, const GUID &category)
{
	if (!pin) return false;

	GUID pinCategory;
	HRESULT hr = GetPinCategory(pin, pinCategory);

	/* if the pin has no category interface, chances are we created it */
	if (FAILED(hr))
		return (hr == E_NOINTERFACE);

	return category == pinCategory;
}

static inline bool PinNameIs(IPin *pin, const wchar_t *name)
{
	if (!pin) return false;
	if (!name) return true;

	PIN_INFO pinInfo;

	if (FAILED(pin->QueryPinInfo(&pinInfo)))
		return false;

	if (pinInfo.pFilter)
		pinInfo.pFilter->Release();

	return wcscmp(name, pinInfo.achName) == 0;
}

static inline bool PinMatches(IPin *pin, const GUID &type, const GUID &category,
		PIN_DIRECTION &dir)
{
	if (!PinHasMajorType(pin, type))
		return false;
	if (!PinIsDirection(pin, dir))
		return false;
	if (!PinIsCategory(pin, category))
		return false;

	return true;
}

bool GetFilterPin(IBaseFilter *filter, const GUID &type, const GUID &category,
		PIN_DIRECTION dir, IPin **pin)
{
	ComPtr<IPin>       curPin;
	ComPtr<IEnumPins>  pinsEnum;
	ULONG              num;

	if (!filter)
		return false;
	if (FAILED(filter->EnumPins(&pinsEnum)))
		return false;

	while (pinsEnum->Next(1, &curPin, &num) == S_OK) {

		if (PinMatches(curPin, type, category, dir)) {
			*pin = curPin;
			(*pin)->AddRef();
			return true;
		}
	}

	return false;
}

bool GetPinByName(IBaseFilter *filter, PIN_DIRECTION dir, const wchar_t *name,
		IPin **pin)
{
	ComPtr<IPin>       curPin;
	ComPtr<IEnumPins>  pinsEnum;
	ULONG              num;

	if (!filter)
		return false;
	if (FAILED(filter->EnumPins(&pinsEnum)))
		return false;

	while (pinsEnum->Next(1, &curPin, &num) == S_OK) {
		wstring pinName;

		if (PinIsDirection(curPin, dir) && PinNameIs(curPin, name)) {
			*pin = curPin.Detach();
			return true;
		}
	}

	return false;
}

bool GetPinByMedium(IBaseFilter *filter, REGPINMEDIUM &medium, IPin **pin)
{
	ComPtr<IPin>       curPin;
	ComPtr<IEnumPins>  pinsEnum;
	ULONG              num;

	if (!filter)
		return false;
	if (FAILED(filter->EnumPins(&pinsEnum)))
		return false;

	while (pinsEnum->Next(1, &curPin, &num) == S_OK) {
		REGPINMEDIUM curMedium;

		if (GetPinMedium(curPin, curMedium) &&
		    memcmp(&medium, &curMedium, sizeof(medium)) == 0) {
			*pin = curPin.Detach();
			return true;
		}
	}

	return false;
}

static bool GetFilterByMediumFromMoniker(IMoniker *moniker,
		REGPINMEDIUM &medium, IBaseFilter **filter)
{
	ComPtr<IBaseFilter>  curFilter;
	HRESULT              hr;

	hr = moniker->BindToObject(nullptr, nullptr, IID_IBaseFilter,
			(void**)&curFilter);
	if (SUCCEEDED(hr)) {
		ComPtr<IPin> pin;
		if (GetPinByMedium(curFilter, medium, &pin)) {
			*filter = curFilter.Detach();
			return true;
		}
	} else {
		WarningHR(L"GetFilterByMediumFromMoniker: BindToObject failed",
				hr);
	}

	return false;
}

bool GetFilterByMedium(const CLSID &id, REGPINMEDIUM &medium,
		IBaseFilter **filter)
{
	ComPtr<ICreateDevEnum>  deviceEnum;
	ComPtr<IEnumMoniker>    enumMoniker;
	ComPtr<IMoniker>        moniker;
	DWORD                   count = 0;
	HRESULT                 hr;

	hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr,
			CLSCTX_INPROC_SERVER, IID_ICreateDevEnum,
			(void**)&deviceEnum);
	if (FAILED(hr)) {
		WarningHR(L"GetFilterByMedium: Failed to create device enum",
				hr);
		return false;
	}

	hr = deviceEnum->CreateClassEnumerator(id, &enumMoniker, 0);
	if (FAILED(hr)) {
		WarningHR(L"GetFilterByMedium: Failed to create enum moniker",
				hr);
		return false;
	}

	enumMoniker->Reset();

	while (enumMoniker->Next(1, &moniker, &count) == S_OK) {
		if (GetFilterByMediumFromMoniker(moniker, medium, filter))
			return true;
	}

	return false;
}

bool GetPinMedium(IPin *pin, REGPINMEDIUM &medium)
{
	ComQIPtr<IKsPin>              ksPin(pin);
	CoTaskMemPtr<KSMULTIPLE_ITEM> items;

	if (!ksPin)
		return false;

	if (FAILED(ksPin->KsQueryMediums(&items)))
		return false;

	REGPINMEDIUM *curMed = reinterpret_cast<REGPINMEDIUM*>(items + 1);
	for (ULONG i = 0; i < items->Count; i++, curMed++) {
		if (!IsEqualGUID(curMed->clsMedium, GUID_NULL) &&
		    !IsEqualGUID(curMed->clsMedium, KSMEDIUMSETID_Standard)) {
			medium = *curMed;
			return true;
		}
	}

	memset(&medium, 0, sizeof(medium));
	return false;
}

static inline bool PinIsConnected(IPin *pin)
{
	ComPtr<IPin> connectedPin;
	return SUCCEEDED(pin->ConnectedTo(&connectedPin));
}

static bool DirectConnectOutputPin(IFilterGraph *graph, IPin *pin,
		IBaseFilter *filterIn)
{
	ComPtr<IPin>       curPin;
	ComPtr<IEnumPins>  pinsEnum;
	ULONG              num;

	if (!graph || !filterIn || !pin)
		return false;
	if (FAILED(filterIn->EnumPins(&pinsEnum)))
		return false;

	while (pinsEnum->Next(1, &curPin, &num) == S_OK) {

		if (PinIsDirection(curPin, PINDIR_INPUT) &&
		    !PinIsConnected(curPin)) {
			if (graph->ConnectDirect(pin, curPin, nullptr) == S_OK)
				return true;
		}
	}

	return false;
}

bool DirectConnectFilters(IFilterGraph *graph, IBaseFilter *filterOut,
		IBaseFilter *filterIn)
{
	ComPtr<IPin>       curPin;
	ComPtr<IEnumPins>  pinsEnum;
	ULONG              num;
	bool               connected = false;

	if (!graph || !filterOut || !filterIn)
		return false;
	if (FAILED(filterOut->EnumPins(&pinsEnum)))
		return false;

	while (pinsEnum->Next(1, &curPin, &num) == S_OK) {

		if (PinIsDirection(curPin, PINDIR_OUTPUT) &&
		    !PinIsConnected(curPin)) {
			if (DirectConnectOutputPin(graph, curPin, filterIn))
				connected = true;
		}
	}

	return connected;
}

HRESULT MapPinToPacketID(IPin *pin, ULONG packetID)
{
	ComQIPtr<IMPEG2PIDMap> pidMap(pin);
	if (!pidMap)
		return E_NOINTERFACE;

	return pidMap->MapPID(1, &packetID, MEDIA_ELEMENTARY_STREAM);
}

wstring ConvertHRToEnglish(HRESULT hr)
{
	LPWSTR buffer = NULL;
	wstring str;

	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, hr, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
			(LPTSTR)&buffer, 0, NULL);

	if (buffer) {
		str = buffer;
		LocalFree(buffer);
	}

	return str.c_str();
}

}; /* namespace DShow */
