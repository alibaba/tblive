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

#include "encoder.hpp"
#include "log.hpp"
#include "avermedia-encode.h"

namespace DShow {

HVideoEncoder::HVideoEncoder()
{
	initialized = CreateFilterGraph(&graph, &builder, &control);
}

HVideoEncoder::~HVideoEncoder()
{
	ComPtr<IEnumFilters> filterEnum;
	IBaseFilter *filter;
	HRESULT hr;

	if (!initialized)
		return;

	if (active)
		control->Stop();

	/* seems like you have to manually release the entire graph otherwise
	 * the encoder device might not end up releasing properly */
	hr = graph->EnumFilters(&filterEnum);
	if (hr == S_OK) {
		while (filterEnum->Next(1, &filter, nullptr) == S_OK) {
			graph->RemoveFilter(filter);
			filterEnum->Reset();
			filter->Release();
		}
	}
}

bool HVideoEncoder::ConnectFilters()
{
	ComPtr<IPin> deviceIn;
	ComPtr<IPin> deviceOut;
	ComPtr<IPin> encoderIn;
	ComPtr<IPin> encoderOut;
	bool success;
	HRESULT hr;

	success = GetPinByName(device, PINDIR_INPUT, L"YUV In", &deviceIn);
	if (!success) {
		Warning(L"Failed to get YUV In pin");
		return false;
	}

	success = GetPinByName(device, PINDIR_OUTPUT, L"Virtual Video Out",
			&deviceOut);
	if (!success) {
		Warning(L"Failed to get Virtual Video Out pin");
		return false;
	}

	success = GetPinByName(encoder, PINDIR_INPUT, L"Virtual Video In",
			&encoderIn);
	if (!success) {
		Warning(L"Failed to get encoder input pin");
		return false;
	}

	success = GetPinByName(encoder, PINDIR_OUTPUT, nullptr, &encoderOut);
	if (!success) {
		Warning(L"Failed to get encoder output pin");
		return false;
	}

	hr = graph->ConnectDirect(output->GetPin(), deviceIn, nullptr);
	if (FAILED(hr)) {
		WarningHR(L"Failed to connect output to device", hr);
		return false;
	}

	hr = graph->ConnectDirect(deviceOut, encoderIn, nullptr);
	if (FAILED(hr)) {
		WarningHR(L"Failed to connect device to encoder", hr);
		return false;
	}

	hr = graph->ConnectDirect(encoderOut, capture->GetPin(), nullptr);
	if (FAILED(hr)) {
		WarningHR(L"Failed to connect encoder to capture", hr);
		return false;
	}

	return true;
}

static bool GetPinFirstMediaType(IPin *pin, AM_MEDIA_TYPE **mt)
{
	ComPtr<IEnumMediaTypes>        mediaEnum;
	HRESULT                        hr;
	ULONG                          fetched;

	hr = pin->EnumMediaTypes(&mediaEnum);
	if (FAILED(hr)) {
		Warning(L"Failed to get pin media type enum");
		return false;
	}

	if (mediaEnum->Next(1, mt, &fetched) != S_OK) {
		Warning(L"Failed to get pin media type");
		return false;
	}

	return true;
}

bool HVideoEncoder::SetupCrossbar()
{
	ComPtr<IBaseFilter> crossbar;
	ComPtr<IPin> pin;
	REGPINMEDIUM medium;

	/* C353 has no crossbar */
	if (config.name.find(L"C353") != std::string::npos)
		return true;

	if (!GetPinByName(device, PINDIR_INPUT, L"Analog Video In", &pin)) {
		Warning(L"Failed to get Analog Video In pin");
		return false;
	}
	if (!GetPinMedium(pin, medium)) {
		Warning(L"Failed to get Analog Video In pin medium");
		return false;
	}
	if (!GetFilterByMedium(AM_KSCATEGORY_CROSSBAR, medium, &crossbar)) {
		Warning(L"Failed to get crossbar filter");
		return false;
	}

	graph->AddFilter(crossbar, L"Crossbar Filter");

	if (!DirectConnectFilters(graph, crossbar, device)) {
		Warning(L"Failed to connect crossbar to device");
		return false;
	}

	return true;
}

void HVideoEncoder::InitializeVideoFormat(MediaType &mt)
{
	long long frameTime;
	DWORD size;
	long long bitrate;

	frameTime = config.fpsDenominator;
	frameTime *= 10000000;
	frameTime /= config.fpsNumerator;

	size = config.cx * config.cy;
	size += size / 2;

	bitrate = size * config.fpsNumerator / config.fpsDenominator;

	VIDEOINFOHEADER *vih           = mt.AllocFormat<VIDEOINFOHEADER>();
	vih->bmiHeader.biSize          = sizeof(vih->bmiHeader);
	vih->bmiHeader.biWidth         = config.cx;
	vih->bmiHeader.biHeight        = config.cy;
	vih->bmiHeader.biPlanes        = 1;
	vih->bmiHeader.biBitCount      = 12;
	vih->bmiHeader.biSizeImage     = size;
	vih->bmiHeader.biCompression   = MAKEFOURCC('Y', 'V', '1', '2');
	vih->rcSource.right            = config.cx;
	vih->rcSource.bottom           = config.cy;
	vih->rcTarget                  = vih->rcSource;
	vih->dwBitRate                 = (DWORD)(bitrate * 8);
	vih->AvgTimePerFrame           = frameTime;

	mt->majortype                  = MEDIATYPE_Video;
	mt->subtype                    = MEDIASUBTYPE_YV12;
	mt->formattype                 = FORMAT_VideoInfo;
	mt->bFixedSizeSamples          = true;
	mt->lSampleSize                = size;
}

bool HVideoEncoder::SetupEncoder(IBaseFilter *filter)
{
	ComPtr<IBaseFilter>            deviceFilter;
	ComPtr<IPin>                   inputPin;
	ComPtr<IPin>                   outputPin;
	REGPINMEDIUM                   medium;
	MediaTypePtr                   mtRaw;
	MediaTypePtr                   mtEncoded;

	if (!GetPinByName(filter, PINDIR_INPUT, nullptr, &inputPin)) {
		Warning(L"Could not get encoder input pin");
		return false;
	}
	if (!GetPinByName(filter, PINDIR_OUTPUT, nullptr, &outputPin)) {
		Warning(L"Could not get encoder output pin");
		return false;
	}
	if (!GetPinMedium(inputPin, medium)) {
		Warning(L"Could not get input pin medium");
		return false;
	}

	inputPin.Release();

	if (!GetFilterByMedium(CLSID_VideoInputDeviceCategory, medium,
				&deviceFilter)) {
		Warning(L"Could not get device filter from medium");
		return false;
	}
	if (!GetPinByName(deviceFilter, PINDIR_INPUT, L"YUV In", &inputPin)) {
		Warning(L"Could not device YUV pin");
		return false;
	}
	if (!GetPinFirstMediaType(inputPin, &mtRaw)) {
		Warning(L"Could not get YUV pin media type");
		return false;
	}
	if (!GetPinFirstMediaType(outputPin, &mtEncoded)) {
		Warning(L"Could not get encoder output pin media type");
		return false;
	}

	PinCaptureInfo captureInfo;
	captureInfo.callback           = [this] (IMediaSample *s) {Receive(s);};
	captureInfo.expectedMajorType  = mtEncoded->majortype;
	captureInfo.expectedSubType    = mtEncoded->subtype;

	PinOutputInfo outputInfo;
	outputInfo.expectedMajorType   = mtRaw->majortype;
	outputInfo.expectedSubType     = mtRaw->subtype;
	outputInfo.cx                  = config.cx;
	outputInfo.cy                  = config.cy;

	InitializeVideoFormat(outputInfo.mt);

	encoder = filter;
	device  = deviceFilter;
	capture = new CaptureFilter(captureInfo);
	output  = new OutputFilter(outputInfo);

	graph->AddFilter(output, nullptr);
	graph->AddFilter(device, L"Device Filter");
	graph->AddFilter(encoder, L"Encoder Filter");
	graph->AddFilter(capture, nullptr);
	return true;
}

static inline void Clamp(ULONG &val, ULONG minVal, ULONG maxVal)
{
	if (val < minVal) val = minVal;
	else if (val > maxVal) val = maxVal;
}

HRESULT SetAVMEncoderSetting(IKsPropertySet *propertySet, ULONG setting,
		ULONG param1, ULONG param2)
{
	AVER_PARAMETERS params = {};
	params.ulIndex = setting;
	params.ulParam1 = param1;
	params.ulParam2 = param2;

	if (setting == AVER_PARAMETER_ENCODE_FRAME_RATE) {
		Clamp(param1, 15, 60);
	} else if (setting == AVER_PARAMETER_ENCODE_BIT_RATE) {
		Clamp(param1, 1000, 60000);
	} else if (setting == AVER_PARAMETER_CURRENT_RESOLUTION) {
		Clamp(param1, 1, 30);
	}

	return propertySet->Set(AVER_HW_ENCODE_PROPERTY,
			PROPERTY_HW_ENCODE_PARAMETER,
			&params, sizeof(params),
			&params, sizeof(params));
}

bool SetAvermediaEncoderConfig(IBaseFilter *encoder, VideoEncoderConfig &config)
{
	HRESULT hr;

	ComQIPtr<IKsPropertySet> propertySet(encoder);
	if (!propertySet) {
		Warning(L"Could not get IKsPropertySet for encoder");
		return false;
	}

	double fps = double(config.fpsNumerator) /
		double(config.fpsDenominator);

	hr = SetAVMEncoderSetting(propertySet,
			AVER_PARAMETER_ENCODE_FRAME_RATE,
			ULONG(fps), 0);
	if (FAILED(hr)) {
		WarningHR(L"Failed to set Avermedia encoder FPS", hr);
		return false;
	}

	hr = SetAVMEncoderSetting(propertySet,
			AVER_PARAMETER_ENCODE_BIT_RATE,
			ULONG(config.bitrate), 0);
	if (FAILED(hr)) {
		WarningHR(L"Failed to set Avermedia encoder bitrate", hr);
		return false;
	}

	hr = SetAVMEncoderSetting(propertySet,
			AVER_PARAMETER_CURRENT_RESOLUTION,
			ULONG(config.cx), ULONG(config.cy));
	if (FAILED(hr)) {
		WarningHR(L"Failed to set Avermedia encoder current res", hr);
		return false;
	}

	hr = SetAVMEncoderSetting(propertySet,
			AVER_PARAMETER_ENCODE_RESOLUTION,
			ULONG(config.cx), ULONG(config.cy));
	if (FAILED(hr)) {
		WarningHR(L"Failed to set Avermedia encoder res", hr);
		return false;
	}

	hr = SetAVMEncoderSetting(propertySet,
			AVER_PARAMETER_ENCODE_GOP,
			ULONG(config.keyframeInterval), 0);
	if (FAILED(hr)) {
		WarningHR(L"Failed to set Avermedia encoder GOP", hr);
		return false;
	}

	return true;
}

bool HVideoEncoder::SetConfig(VideoEncoderConfig &config)
{
	ComPtr<IBaseFilter> filter;
	ComPtr<IBaseFilter> crossbar;

	if (config.name.empty() && config.path.empty()) {
		Warning(L"No video encoder name or path specified");
		return false;
	}

	bool success = GetDeviceFilter(KSCATEGORY_ENCODER,
			config.name.c_str(), config.path.c_str(), &filter);
	if (!success) {
		Warning(L"Video encoder '%s': %s not found", config.name.c_str(),
				config.path.c_str());
		return false;
	}

	if (!filter) {
		Warning(L"Could not get encoder filter");
		return false;
	}

	this->config = config;

	if (!SetupEncoder(filter)) {
		Warning(L"Failed to set up encoder");
		return false;
	}
	if (!SetupCrossbar()) {
		Warning(L"Failed to set up crossbar");
		return false;
	}
	if (!SetAvermediaEncoderConfig(device, config)) {
		Warning(L"Failed to set Avermedia encoder settings");
		return false;
	}
	if (!ConnectFilters()) {
		Warning(L"Failed to connect encoder filters");
		return false;
	}

	LogFilters(graph);

	HRESULT hr = control->Run();
	if (FAILED(hr)) {
		WarningHR(L"Run failed", hr);
		return false;
	}

	active = true;
	return true;
}

void HVideoEncoder::Receive(IMediaSample *s)
{
	BYTE *data;
	size_t size;

	if (FAILED(s->GetPointer(&data)))
		return;

	size = (size_t)s->GetActualDataLength();
	if (!size)
		return;

	packetMutex.lock();
	packets.emplace_back(data, size);
	packetMutex.unlock();
}

bool HVideoEncoder::Encode(unsigned char *data[DSHOW_MAX_PLANES],
		size_t linesize[DSHOW_MAX_PLANES],
		long long timestampStart, long long timestampEnd,
		EncoderPacket &packet, bool &new_packet)
{
	new_packet = false;

	if (!active)
		return false;

	output->Send(data, linesize, timestampStart, timestampEnd);
	ptsVals.push_back(timestampStart);

	packetMutex.lock();
	if (packets.size() > 0) {
		curPacket = move(packets.front());
		long long ptsOut = ptsVals[0];
		packets.pop_front();
		ptsVals.pop_front();

		packet.data = curPacket.data.data();
		packet.size = curPacket.data.size();
		packet.pts  = ptsOut;
		packet.dts  = ptsOut;
		new_packet  = true;
	}
	packetMutex.unlock();
	return true;
}

};
