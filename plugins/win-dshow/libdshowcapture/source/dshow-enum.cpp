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

#include <stdlib.h>
#include <mutex>
#include "dshow-enum.hpp"
#include "dshow-formats.hpp"
#include "log.hpp"

#undef DEFINE_GUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID DECLSPEC_SELECTANY name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

#include "IVideoCaptureFilter.h"

namespace DShow {
using namespace std;

typedef bool (*EnumCapsCallback)(void *param, const AM_MEDIA_TYPE &mt,
		const BYTE *data);

static void EnumElgatoCaps(IPin *pin, EnumCapsCallback callback, void *param)
{
	ComPtr<IEnumMediaTypes> mediaTypes;

	if (SUCCEEDED(pin->EnumMediaTypes(&mediaTypes))) {
		MediaTypePtr mt;
		ULONG        count = 0;

		while (mediaTypes->Next(1, &mt, &count) == S_OK) {
			if (!callback(param, *mt, nullptr))
				break;
		}
	}
}

static bool EnumPinCaps(IPin *pin, EnumCapsCallback callback, void *param)
{
	HRESULT hr;
	ComQIPtr<IAMStreamConfig> config(pin);
	int count, size;

	if (config == NULL)
		return false;

	hr = config->GetNumberOfCapabilities(&count, &size);
	if (SUCCEEDED(hr)) {
		vector<BYTE> caps;
		caps.resize(size);

		for (int i = 0; i < count; i++) {
			MediaTypePtr mt;
			hr = config->GetStreamCaps(i, &mt, caps.data());
			if (SUCCEEDED(hr))
				if (!callback(param, *mt, caps.data()))
					break;
		}
	} else if (hr == E_NOTIMPL) {
		EnumElgatoCaps(pin, callback, param);
	} else {
		return false;
	}

	return true;
}

/* Note:  DEVICE_VideoInfo is not to be confused with Device::VideoInfo */
static bool Get_FORMAT_VideoInfo_Data(VideoInfo &info,
		const AM_MEDIA_TYPE &mt, const BYTE *data)
{
	const VIDEO_STREAM_CONFIG_CAPS *vscc;
	const VIDEOINFOHEADER          *viHeader;
	const BITMAPINFOHEADER         *bmiHeader;
	VideoFormat                    format;

	vscc      = reinterpret_cast<const VIDEO_STREAM_CONFIG_CAPS*>(data);
	viHeader  = reinterpret_cast<const VIDEOINFOHEADER*>(mt.pbFormat);
	bmiHeader = &viHeader->bmiHeader;

	if (!GetMediaTypeVFormat(mt, format))
		return false;

	info.format = format;

	if (vscc) {
		info.minInterval = vscc->MinFrameInterval;
		info.maxInterval = vscc->MaxFrameInterval;
		info.minCX       = vscc->MinOutputSize.cx;
		info.minCY       = vscc->MinOutputSize.cy;
		info.maxCX       = vscc->MaxOutputSize.cx;
		info.maxCY       = vscc->MaxOutputSize.cy;

		if (!info.minCX || !info.minCY ||
		    !info.maxCX || !info.maxCY) {
			info.minCX = info.maxCX = bmiHeader->biWidth;
			info.minCY = info.maxCY = bmiHeader->biHeight;
		}

		info.granularityCX = max(vscc->OutputGranularityX, 1);
		info.granularityCY = max(vscc->OutputGranularityY, 1);
	} else {
		info.minInterval = info.maxInterval = 10010000000LL/60000LL;
		info.minCX = info.maxCX = bmiHeader->biWidth;
		info.minCY = info.maxCY = bmiHeader->biHeight;
		info.granularityCX = 1;
		info.granularityCY = 1;
	}

	return true;
}

static void Get_FORMAT_WaveFormatEx_Data(AudioInfo &info,
		const AM_MEDIA_TYPE &mt, const BYTE *data)
{
	const AUDIO_STREAM_CONFIG_CAPS *ascc;
	const WAVEFORMATEX             *wfex;

	ascc = reinterpret_cast<const AUDIO_STREAM_CONFIG_CAPS*>(data);
	wfex = reinterpret_cast<const WAVEFORMATEX*>(mt.pbFormat);

	switch (wfex->wBitsPerSample) {
	case 16: info.format = AudioFormat::Wave16bit; break;
	case 32: info.format = AudioFormat::WaveFloat; break;
	}

	info.minChannels           = ascc->MinimumChannels;
	info.maxChannels           = ascc->MaximumChannels;
	info.channelsGranularity   = ascc->ChannelsGranularity;
	info.minSampleRate         = ascc->MinimumSampleFrequency;
	info.maxSampleRate         = ascc->MaximumSampleFrequency;
	info.sampleRateGranularity = ascc->SampleFrequencyGranularity;
}

struct ClosestVideoData {
	VideoConfig &config;
	MediaType   &mt;
	long long   bestVal;
	bool        found;

	ClosestVideoData &operator=(ClosestVideoData const&) = delete;
	ClosestVideoData &operator=(ClosestVideoData&&) = delete;

	inline ClosestVideoData(VideoConfig &config, MediaType &mt)
		: config     (config),
		  mt         (mt),
		  bestVal    (0),
		  found      (false)
	{}
};

static inline void ClampToGranularity(LONG &val, int minVal, int granularity)
{
	val -= ((val - minVal) % granularity);
}

static inline int GetFormatRating(VideoFormat format)
{
	if (format >= VideoFormat::I420 && format < VideoFormat::YVYU)
		return 0;
	else if (format >= VideoFormat::YVYU && format < VideoFormat::MJPEG)
		return 5;
	else if (format >= VideoFormat::MJPEG)
		return 10;

	return 15;
}

static bool ClosestVideoMTCallback(ClosestVideoData &data,
		const AM_MEDIA_TYPE &mt, const BYTE *capData)
{
	VideoInfo info;

	if (mt.formattype == FORMAT_VideoInfo) {
		if (!Get_FORMAT_VideoInfo_Data(info, mt, capData))
			return true;
	} else {
		return true;
	}

	MediaType           copiedMT = mt;
	VIDEOINFOHEADER     *vih     = (VIDEOINFOHEADER*)copiedMT->pbFormat;
	BITMAPINFOHEADER    *bmih    = GetBitmapInfoHeader(copiedMT);

	if (data.config.internalFormat != VideoFormat::Any &&
	    data.config.internalFormat != info.format)
		return true;

	int                 xVal      = 0;
	int                 yVal      = 0;
	int                 formatVal = 0;
	long long           frameVal  = 0;

	if (data.config.cx < info.minCX)
		xVal = info.minCX - data.config.cx;
	else if (data.config.cx > info.maxCX)
		xVal = data.config.cx - info.maxCX;

	if (data.config.cy < info.minCY)
		yVal = info.minCY - data.config.cy;
	else if (data.config.cy > info.maxCY)
		yVal = data.config.cy - info.maxCY;

	if (data.config.frameInterval < info.minInterval)
		frameVal = info.minInterval - data.config.frameInterval;
	else if (data.config.frameInterval > info.maxInterval)
		frameVal = data.config.frameInterval - info.maxInterval;

	formatVal = GetFormatRating(info.format);

	long long totalVal = frameVal + yVal + xVal + formatVal;

	if (!data.found || data.bestVal > totalVal) {
		if (xVal == 0) {
			bmih->biWidth = data.config.cx;
			ClampToGranularity(bmih->biWidth, info.minCX,
					info.granularityCX);
		}

		if (yVal == 0) {
			bmih->biHeight = data.config.cy;
			ClampToGranularity(bmih->biHeight, info.minCY,
					info.granularityCY);
		}

		if (frameVal == 0)
			vih->AvgTimePerFrame = data.config.frameInterval;

		data.found   = true;
		data.bestVal = totalVal;
		data.mt      = copiedMT;

		if (totalVal == 0)
			return false;
	}

	return true;
}

bool GetClosestVideoMediaType(IBaseFilter *filter, VideoConfig &config,
		MediaType &mt)
{
	ComPtr<IPin>     pin;
	ClosestVideoData data(config, mt);
	bool             success;

	success = GetFilterPin(filter, MEDIATYPE_Video, PIN_CATEGORY_CAPTURE,
			PINDIR_OUTPUT, &pin);
	if (!success || pin == NULL) {
		Error(L"GetClosestVideoMediaType: Could not get pin");
		return false;
	}

	success = EnumPinCaps(pin, EnumCapsCallback(ClosestVideoMTCallback),
			&data);
	if (!success) {
		Error(L"GetClosestVideoMediaType: Could not enumerate caps");
		return false;
	}

	return data.found;
}

struct ClosestAudioData {
	AudioConfig &config;
	MediaType   &mt;
	int         bestVal;
	bool        found;

	ClosestAudioData &operator=(ClosestAudioData const&) = delete;
	ClosestAudioData &operator=(ClosestAudioData&&) = delete;

	inline ClosestAudioData(AudioConfig &config, MediaType &mt)
		: config     (config),
		  mt         (mt),
		  bestVal    (0),
		  found      (false)
	{}
};

static bool ClosestAudioMTCallback(ClosestAudioData &data,
		const AM_MEDIA_TYPE &mt, const BYTE *capData)
{
	AudioInfo info = {};

	if (mt.formattype == FORMAT_WaveFormatEx)
		Get_FORMAT_WaveFormatEx_Data(info, mt, capData);
	else
		return true;

	MediaType    copiedMT = mt;
	WAVEFORMATEX *wfex    = (WAVEFORMATEX*)copiedMT->pbFormat;

	if (data.config.format != AudioFormat::Any &&
	    data.config.format != info.format)
		return true;

	int sampleRateVal = 0;
	int channelsVal   = 0;

	if (data.config.sampleRate < info.minSampleRate)
		sampleRateVal = info.minSampleRate - data.config.sampleRate;
	else if (data.config.sampleRate > info.maxSampleRate)
		sampleRateVal = data.config.sampleRate - info.maxSampleRate;

	if (data.config.channels < info.minChannels)
		channelsVal = info.minChannels - data.config.channels;
	else if (info.maxChannels > data.config.channels)
		channelsVal = data.config.channels - info.maxChannels;

	int totalVal = sampleRateVal + channelsVal;

	if (!data.found || data.bestVal > totalVal) {
		if (channelsVal == 0) {
			LONG channels = data.config.channels;
			ClampToGranularity(channels, info.minChannels,
					info.channelsGranularity);
			wfex->nChannels = (WORD)channels;

			wfex->nBlockAlign =
				wfex->wBitsPerSample * wfex->nChannels / 8;
		}

		if (sampleRateVal == 0) {
			wfex->nSamplesPerSec = data.config.sampleRate;
			ClampToGranularity((LONG&)wfex->nSamplesPerSec,
					info.minSampleRate,
					info.sampleRateGranularity);
		}

		wfex->nAvgBytesPerSec =
			wfex->nSamplesPerSec * wfex->nBlockAlign;

		data.mt      = copiedMT;
		data.found   = true;
		data.bestVal = totalVal;

		if (totalVal == 0)
			return false;
	}

	return true;
}

bool GetClosestAudioMediaType(IBaseFilter *filter, AudioConfig &config,
		MediaType &mt)
{
	ComPtr<IPin>     pin;
	ClosestAudioData data(config, mt);
	bool             success;

	success = GetFilterPin(filter, MEDIATYPE_Audio, PIN_CATEGORY_CAPTURE,
			PINDIR_OUTPUT, &pin);
	if (!success || pin == NULL) {
		Error(L"GetClosestAudioMediaType: Could not get pin");
		return false;
	}

	success = EnumPinCaps(pin, EnumCapsCallback(ClosestAudioMTCallback),
			&data);
	if (!success) {
		Error(L"GetClosestAudioMediaType: Could not enumerate caps");
		return false;
	}

	return data.found;
}

static bool EnumVideoCap(vector<VideoInfo> &caps,
		const AM_MEDIA_TYPE &mt, const BYTE *data)
{
	VideoInfo info;

	if (mt.formattype == FORMAT_VideoInfo)
		if (Get_FORMAT_VideoInfo_Data(info, mt, data))
			caps.push_back(info);

	return true;
}

bool EnumVideoCaps(IPin *pin, vector<VideoInfo> &caps)
{
	return EnumPinCaps(pin, EnumCapsCallback(EnumVideoCap), &caps);
}

static bool EnumAudioCap(vector<AudioInfo> &caps,
		const AM_MEDIA_TYPE &mt, const BYTE *data)
{
	AudioInfo info;

	if (mt.formattype == FORMAT_WaveFormatEx) {
		Get_FORMAT_WaveFormatEx_Data(info, mt, data);
		caps.push_back(info);
	}

	return true;
}

bool EnumAudioCaps(IPin *pin, vector<AudioInfo> &caps)
{
	return EnumPinCaps(pin, EnumCapsCallback(EnumAudioCap), &caps);
}

static bool decklinkVideoPresent = false;

static bool EnumDevice(const GUID &type, IMoniker *deviceInfo,
		EnumDeviceCallback callback, void *param)
{
	ComPtr<IPropertyBag> propertyData;
	ComPtr<IBaseFilter>  filter;
	HRESULT hr;

	hr = deviceInfo->BindToStorage(0, 0, IID_IPropertyBag,
			(void**)&propertyData);
	if (FAILED(hr))
		return true;

	VARIANT deviceName, devicePath;
	deviceName.vt      = VT_BSTR;
	devicePath.vt      = VT_BSTR;
	devicePath.bstrVal = NULL;

	hr = propertyData->Read(L"FriendlyName", &deviceName, NULL);
	if (FAILED(hr))
		return true;

	/* workaround to a crash in decklink drivers; if no decklink device
	 * is plugged in to the system, it will still try to enumerate the
	 * decklink audio device, but will crash when trying to bind it to
	 * a filter due to a bug in the drivers */
	if (deviceName.bstrVal &&
	    type == CLSID_AudioInputDeviceCategory &&
	    wcsstr(deviceName.bstrVal, L"Decklink") != nullptr &&
	    !decklinkVideoPresent) {
		return true;
	}

	propertyData->Read(L"DevicePath", &devicePath, NULL);

	hr = deviceInfo->BindToObject(NULL, 0, IID_IBaseFilter,
			(void**)&filter);
	if (SUCCEEDED(hr)) {
		if (!callback(param, filter, deviceName.bstrVal,
				devicePath.bstrVal))
			return false;
	}

	return true;
}

static bool EnumExceptionVideoDevices(EnumDeviceCallback callback, void *param)
{
	ComPtr<IBaseFilter>  filter;
	HRESULT              hr;

	hr = CoCreateInstance(CLSID_ElgatoVideoCaptureFilter, nullptr,
			CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&filter);
	if (SUCCEEDED(hr)) {
		if (!callback(param, filter, L"Elgato Game Capture HD",
					L"__elgato"))
			return false;
	}

	return true;
}

static recursive_mutex enumMutex;

static bool CheckForDLCallback(void *unused,
		IBaseFilter *filter,
		const wchar_t *deviceName,
		const wchar_t *devicePath)
{
	if (wcsstr(deviceName, L"Decklink") != nullptr) {
		decklinkVideoPresent = true;
		return false;
	}

	DSHOW_UNUSED(unused);
	DSHOW_UNUSED(filter);
	DSHOW_UNUSED(devicePath);
	return true;
}

static void CheckForDecklinkVideo()
{
	decklinkVideoPresent = false;
	EnumDevices(CLSID_VideoInputDeviceCategory, CheckForDLCallback,
			nullptr);
}

bool EnumDevices(const GUID &type, EnumDeviceCallback callback, void *param)
{
	lock_guard<recursive_mutex> lock(enumMutex);
	ComPtr<ICreateDevEnum>  deviceEnum;
	ComPtr<IEnumMoniker>    enumMoniker;
	ComPtr<IMoniker>        deviceInfo;
	HRESULT                 hr;
	DWORD                   count = 0;

	if (type == CLSID_AudioInputDeviceCategory) {
		CheckForDecklinkVideo();
	}

	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
		CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&deviceEnum);
	if (FAILED(hr)) {
		WarningHR(L"EnumAudioDevices: Could not create "
		          L"ICreateDeviceEnum", hr);
		return false;
	}

	hr = deviceEnum->CreateClassEnumerator(type, &enumMoniker, 0);
	if (FAILED(hr)) {
		WarningHR(L"EnumAudioDevices: CreateClassEnumerator failed",
				hr);
		return false;
	}

	if (hr == S_OK) {
		while (enumMoniker->Next(1, &deviceInfo, &count) == S_OK) {
			if (!EnumDevice(type, deviceInfo, callback, param))
				return true;
		}
	}

	if (type == CLSID_VideoInputDeviceCategory)
		if (!EnumExceptionVideoDevices(callback, param))
			return true;

	return true;
}

}; /* namespace DShow */
