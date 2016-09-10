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

#include "device.hpp"
#include "dshow-device-defs.hpp"
#include "dshow-media-type.hpp"
#include "dshow-formats.hpp"
#include "dshow-enum.hpp"
#include "log.hpp"

#define ROCKET_WAIT_TIME_MS 5000

namespace DShow {

bool SetRocketEnabled(IBaseFilter *encoder, bool enable);

HDevice::HDevice()
	: initialized (false),
	  active      (false)
{
}

HDevice::~HDevice()
{
	if (active)
		Stop();

	DisconnectFilters();

	/*
	 * the sleeps for the rocket are required.  It seems that you cannot
	 * simply start/stop the stream right away after/before you enable or
	 * disable the rocket.  If you start it too fast after enabling, it
	 * won't return any data.  If you try to turn off the rocket too
	 * quickly after stopping, then it'll be perpetually stuck on, and then
	 * you'll have to unplug/replug the device to get it working again.
	 */
	if (!!rocketEncoder) {
		Sleep(ROCKET_WAIT_TIME_MS);
		SetRocketEnabled(rocketEncoder, false);
	}
}

bool HDevice::EnsureInitialized(const wchar_t *func)
{
	if (!initialized) {
		Error(L"%s: context not initialized", func);
		return false;
	}

	return true;
}

bool HDevice::EnsureActive(const wchar_t *func)
{
	if (!active) {
		Error(L"%s: cannot be used while inactive", func);
		return false;
	}

	return true;
}

bool HDevice::EnsureInactive(const wchar_t *func)
{
	if (active) {
		Error(L"%s: cannot be used while active", func);
		return false;
	}

	return true;
}

inline void HDevice::SendToCallback(bool video,
		unsigned char *data, size_t size,
		long long startTime, long long stopTime)
{
	if (!size)
		return;

	if (video)
		videoConfig.callback(videoConfig, data, size, startTime,
				stopTime);
	else
		audioConfig.callback(audioConfig, data, size, startTime,
				stopTime);
}

void HDevice::Receive(bool isVideo, IMediaSample *sample)
{
	BYTE *ptr;
	MediaTypePtr mt;
	bool encoded = isVideo ?
		((int)videoConfig.format >= 400) :
		((int)audioConfig.format >= 200);

	if (!sample)
		return;

	if (isVideo ? !videoConfig.callback : !audioConfig.callback)
		return;

	if (sample->GetMediaType(&mt) == S_OK) {
		if (isVideo) {
			videoMediaType = mt;
			ConvertVideoSettings();
		} else {
			audioMediaType = mt;
			ConvertAudioSettings();
		}
	}

	int size = sample->GetActualDataLength();
	if (!size)
		return;

	if (FAILED(sample->GetPointer(&ptr)))
		return;

	long long startTime, stopTime;
	bool hasTime = SUCCEEDED(sample->GetTime(&startTime, &stopTime));

	if (encoded) {
		EncodedData &data = isVideo ? encodedVideo : encodedAudio;

		/* packets that have time are the first packet in a group of
		 * segments */
		if (hasTime) {
			SendToCallback(isVideo,
					data.bytes.data(), data.bytes.size(),
					data.lastStartTime, data.lastStopTime);

			data.bytes.resize(0);
			data.lastStartTime = startTime;
			data.lastStopTime  = stopTime;
		}

		data.bytes.insert(data.bytes.end(),
				(unsigned char*)ptr,
				(unsigned char*)ptr + size);

	} else if (hasTime) {
		SendToCallback(isVideo, ptr, size, startTime, stopTime);
	}
}

void HDevice::ConvertVideoSettings()
{
	VIDEOINFOHEADER  *vih  = (VIDEOINFOHEADER*)videoMediaType->pbFormat;
	BITMAPINFOHEADER *bmih = GetBitmapInfoHeader(videoMediaType);

	if (bmih) {
		Debug(L"Video media type changed");

		videoConfig.cx            = bmih->biWidth;
		videoConfig.cy            = bmih->biHeight;
		videoConfig.frameInterval = vih->AvgTimePerFrame;

		bool same = videoConfig.internalFormat == videoConfig.format;
		GetMediaTypeVFormat(videoMediaType, videoConfig.internalFormat);

		if (same)
			videoConfig.format = videoConfig.internalFormat;
	}
}

void HDevice::ConvertAudioSettings()
{
	WAVEFORMATEX *wfex =
		reinterpret_cast<WAVEFORMATEX*>(audioMediaType->pbFormat);

	Debug(L"Audio media type changed");

	audioConfig.sampleRate = wfex->nSamplesPerSec;
	audioConfig.channels   = wfex->nChannels;

	if (wfex->wFormatTag == WAVE_FORMAT_RAW_AAC1)
		audioConfig.format = AudioFormat::AAC;
	else if (wfex->wFormatTag == WAVE_FORMAT_DVM)
		audioConfig.format = AudioFormat::AC3;
	else if (wfex->wFormatTag == WAVE_FORMAT_MPEG)
		audioConfig.format = AudioFormat::MPGA;
	else if (wfex->wBitsPerSample == 16)
		audioConfig.format = AudioFormat::Wave16bit;
	else if (wfex->wBitsPerSample == 32)
		audioConfig.format = AudioFormat::WaveFloat;
	else
		audioConfig.format = AudioFormat::Unknown;
}

#define HD_PVR1_NAME L"Hauppauge HD PVR Capture"

bool HDevice::SetupExceptionVideoCapture(IBaseFilter *filter,
		VideoConfig &config)
{
	ComPtr<IPin> pin;

	if (GetPinByName(filter, PINDIR_OUTPUT, L"656", &pin))
		return SetupEncodedVideoCapture(filter, config, HD_PVR2);

	else if (GetPinByName(filter, PINDIR_OUTPUT, L"TS Out", &pin))
		return SetupEncodedVideoCapture(filter, config, Roxio);

	return false;
}

static bool GetPinMediaType(IPin *pin, MediaType &mt)
{
	ComPtr<IEnumMediaTypes> mediaTypes;

	if (SUCCEEDED(pin->EnumMediaTypes(&mediaTypes))) {
		MediaTypePtr curMT;
		ULONG        count = 0;

		while (mediaTypes->Next(1, &curMT, &count) == S_OK) {
			if (curMT->formattype == FORMAT_VideoInfo) {
				mt = curMT;
				return true;
			}
		}
	}

	return false;
}

bool HDevice::SetupVideoCapture(IBaseFilter *filter, VideoConfig &config)
{
	ComPtr<IPin>  pin;
	HRESULT       hr;
	bool          success;

	if (config.name.find(L"C875") != std::string::npos ||
	    config.name.find(L"C835") != std::string::npos)
		return SetupEncodedVideoCapture(filter, config, AV_LGP);

	else if (config.name.find(L"IT9910") != std::string::npos)
		return SetupEncodedVideoCapture(filter, config, HD_PVR_Rocket);

	else if (config.name.find(HD_PVR1_NAME) != std::string::npos)
		return SetupEncodedVideoCapture(filter, config, HD_PVR1);

	success = GetFilterPin(filter, MEDIATYPE_Video, PIN_CATEGORY_CAPTURE,
			PINDIR_OUTPUT, &pin);
	if (!success) {
		if (SetupExceptionVideoCapture(filter, config)) {
			return true;
		} else {
			Error(L"Could not get video pin");
			return false;
		}
	}

	ComQIPtr<IAMStreamConfig> pinConfig(pin);
	if (pinConfig == NULL) {
		Error(L"Could not get IAMStreamConfig for device");
		return false;
	}

	if (config.useDefaultConfig) {
		MediaTypePtr defaultMT;

		hr = pinConfig->GetFormat(&defaultMT);
		if (hr == E_NOTIMPL) {
			if (!GetPinMediaType(pin, videoMediaType)) {
				Error(L"Couldn't get pin media type");
				return false;
			}

		} else if (FAILED(hr)) {
			ErrorHR(L"Could not get default format for video", hr);
			return false;

		} else {
			videoMediaType = defaultMT;
		}

		ConvertVideoSettings();

		config.format = config.internalFormat = VideoFormat::Any;
	}

	if (!GetClosestVideoMediaType(filter, config, videoMediaType)) {
		Error(L"Could not get closest video media type");
		return false;
	}

	hr = pinConfig->SetFormat(videoMediaType);
	if (FAILED(hr) && hr != E_NOTIMPL) {
		ErrorHR(L"Could not set video format", hr);
		return false;
	}

	ConvertVideoSettings();

	PinCaptureInfo info;
	info.callback          = [this] (IMediaSample *s) {Receive(true, s);};
	info.expectedMajorType = videoMediaType->majortype;

	/* attempt to force intermediary filters for these types */
	if (videoConfig.format == VideoFormat::XRGB)
		info.expectedSubType = MEDIASUBTYPE_RGB32;
	else if (videoConfig.format == VideoFormat::ARGB)
		info.expectedSubType = MEDIASUBTYPE_ARGB32;
	else if (videoConfig.format == VideoFormat::YVYU)
		info.expectedSubType = MEDIASUBTYPE_YVYU;
	else if (videoConfig.format == VideoFormat::YUY2)
		info.expectedSubType = MEDIASUBTYPE_YUY2;
	else if (videoConfig.format == VideoFormat::UYVY)
		info.expectedSubType = MEDIASUBTYPE_UYVY;
	else
		info.expectedSubType = videoMediaType->subtype;

	videoCapture = new CaptureFilter(info);
	videoFilter  = filter;

	graph->AddFilter(videoCapture, L"Video Capture Filter");
	graph->AddFilter(videoFilter, L"Video Filter");
	return true;
}

bool HDevice::SetVideoConfig(VideoConfig *config)
{
	ComPtr<IBaseFilter> filter;

	if (!EnsureInitialized(L"SetVideoConfig") ||
	    !EnsureInactive(L"SetVideoConfig"))
		return false;

	videoMediaType = NULL;
	graph->RemoveFilter(videoFilter);
	graph->RemoveFilter(videoCapture);
	videoFilter.Release();
	videoCapture.Release();

	if (!config)
		return true;

	if (config->name.empty() && config->path.empty()) {
		Error(L"No video device name or path specified");
		return false;
	}

	bool success = GetDeviceFilter(CLSID_VideoInputDeviceCategory,
			config->name.c_str(), config->path.c_str(), &filter);
	if (!success) {
		Error(L"Video device '%s': %s not found", config->name.c_str(),
				config->path.c_str());
		return false;
	}

	if (filter == NULL) {
		Error(L"Could not get video filter");
		return false;
	}

	videoConfig = *config;

	if (!SetupVideoCapture(filter, videoConfig))
		return false;

	*config = videoConfig;
	return true;
}

bool HDevice::SetupExceptionAudioCapture(IPin *pin)
{
	ComPtr<IEnumMediaTypes>  enumMediaTypes;
	ULONG                    count = 0;
	HRESULT                  hr;
	MediaTypePtr             mt;

	hr = pin->EnumMediaTypes(&enumMediaTypes);
	if (FAILED(hr)) {
		WarningHR(L"SetupExceptionAudioCapture: pin->EnumMediaTypes "
		          L"failed", hr);
		return false;
	}

	enumMediaTypes->Reset();

	if (enumMediaTypes->Next(1, &mt, &count) == S_OK &&
	    mt->formattype == FORMAT_WaveFormatEx) {
		audioMediaType = mt;
		return true;
	}

	return false;
}

bool HDevice::SetupAudioCapture(IBaseFilter *filter, AudioConfig &config)
{
	ComPtr<IPin>  pin;
	MediaTypePtr  defaultMT;
	bool          success;
	HRESULT       hr;

	success = GetFilterPin(filter, MEDIATYPE_Audio, PIN_CATEGORY_CAPTURE,
			PINDIR_OUTPUT, &pin);
	if (!success) {
		Error(L"Could not get audio pin");
		return false;
	}

	ComQIPtr<IAMStreamConfig> pinConfig(pin);

	if (config.useDefaultConfig) {
		MediaTypePtr defaultMT;

		if (pinConfig && SUCCEEDED(pinConfig->GetFormat(&defaultMT))) {
			audioMediaType = defaultMT;
		} else {
			if (!SetupExceptionAudioCapture(pin)) {
				Error(L"Could not get default format for "
				      L"audio pin");
				return false;
			}
		}
	} else {
		if (!GetClosestAudioMediaType(filter, config, audioMediaType)) {
			Error(L"Could not get closest audio media type");
			return false;
		}
	}

	if (!!pinConfig) {
		hr = pinConfig->SetFormat(audioMediaType);

		if (FAILED(hr) && hr != E_NOTIMPL) {
			Error(L"Could not set audio format");
			return false;
		}
	}

	ConvertAudioSettings();

	PinCaptureInfo info;
	info.callback          = [this] (IMediaSample *s) {Receive(false, s);};
	info.expectedMajorType = audioMediaType->majortype;
	info.expectedSubType   = audioMediaType->subtype;

	audioCapture = new CaptureFilter(info);
	audioFilter  = filter;
	audioConfig  = config;

	graph->AddFilter(audioCapture, L"Audio Capture Filter");
	if (!config.useVideoDevice)
		graph->AddFilter(audioFilter, L"Audio Filter");
	return true;
}

bool HDevice::SetupAudioOutput(IBaseFilter *filter, AudioConfig &config)
{
	ComPtr<IBaseFilter> outputFilter;
	const CLSID *clsID;
	HRESULT hr;

	if (config.mode == AudioMode::WaveOut) {
		clsID = &CLSID_AudioRender;
	} else {
		clsID = &CLSID_DSoundRender;
	}

	hr = CoCreateInstance(*clsID, nullptr, CLSCTX_INPROC_SERVER,
			IID_IBaseFilter, (void**)&outputFilter);
	if (FAILED(hr)) {
		ErrorHR(L"Failed to create audio sound output filter", hr);
		return false;
	}

	audioFilter = filter;
	audioOutput = outputFilter;

	graph->AddFilter(audioOutput, L"Audio Output Filter");
	if (!config.useVideoDevice)
		graph->AddFilter(audioFilter, L"Audio Filter");
	return true;
}

bool HDevice::SetAudioConfig(AudioConfig *config)
{
	ComPtr<IBaseFilter> filter;

	if (!EnsureInitialized(L"SetAudioConfig") ||
	    !EnsureInactive(L"SetAudioConfig"))
		return false;

	if (!audioConfig.useVideoDevice)
		graph->RemoveFilter(audioFilter);
	graph->RemoveFilter(audioCapture);
	graph->RemoveFilter(audioOutput);
	audioFilter.Release();
	audioCapture.Release();
	audioOutput.Release();
	audioMediaType = NULL;

	if (!config)
		return true;

	if (!config->useVideoDevice &&
	    config->name.empty() && config->path.empty()) {
		Error(L"No audio device name or path specified");
		return false;
	}

	if (config->useVideoDevice) {
		if (videoFilter == NULL) {
			Error(L"Tried to use video device's built-in audio, "
			      L"but no video device is present");
			return false;
		}

		filter = videoFilter;
	} else {
		bool success = GetDeviceFilter(CLSID_AudioInputDeviceCategory,
				config->name.c_str(), config->path.c_str(),
				&filter);
		if (!success) {
			Error(L"Audio device '%s': %s not found", config->name.c_str(),
					config->path.c_str());
			return false;
		}
	}

	if (filter == NULL)
		return false;

	audioConfig = *config;

	if (config->mode == AudioMode::Capture) {
		if (!SetupAudioCapture(filter, audioConfig))
			return false;

		*config = audioConfig;
		return true;
	}

	return SetupAudioOutput(filter, audioConfig);
}

bool HDevice::CreateGraph()
{
	if (initialized) {
		Warning(L"Graph already created");
		return false;
	}

	if (!CreateFilterGraph(&graph, &builder, &control))
		return false;

	initialized = true;
	return true;
}

bool HDevice::FindCrossbar(IBaseFilter *filter, IBaseFilter **crossbar)
{
	ComPtr<IPin> pin;
	REGPINMEDIUM medium;
	HRESULT hr;

	hr = builder->FindInterface(NULL, NULL, filter, IID_IAMCrossbar,
			(void**)crossbar);
	if (SUCCEEDED(hr))
		return true;

	if (!GetPinByName(filter, PINDIR_INPUT, nullptr, &pin))
		return false;
	if (!GetPinMedium(pin, medium))
		return false;
	if (!GetFilterByMedium(AM_KSCATEGORY_CROSSBAR, medium, crossbar))
		return false;

	graph->AddFilter(*crossbar, L"Crossbar Filter");
	return true;
}

bool HDevice::ConnectPins(const GUID &category, const GUID &type,
		IBaseFilter *filter, IBaseFilter *capture)
{
	HRESULT hr;
	ComPtr<IBaseFilter> crossbar;
	ComPtr<IPin> filterPin;
	ComPtr<IPin> capturePin;
	bool connectCrossbar = !encodedDevice && type == MEDIATYPE_Video;

	if (!EnsureInitialized(L"HDevice::ConnectPins") ||
	    !EnsureInactive(L"HDevice::ConnectPins"))
		return false;

	if (connectCrossbar && FindCrossbar(filter, &crossbar)) {
		if (!DirectConnectFilters(graph, crossbar, filter)) {
			Warning(L"HDevice::ConnectPins: Failed to connect "
			        L"crossbar");
			return false;
		}
	}

	if (!GetFilterPin(filter, type, category, PINDIR_OUTPUT, &filterPin)) {
		Error(L"HDevice::ConnectPins: Failed to find pin");
		return false;
	}

	if (!GetPinByName(capture, PINDIR_INPUT, nullptr, &capturePin)) {
		Error(L"HDevice::ConnectPins: Failed to find capture pin");
		return false;
	}

	hr = graph->ConnectDirect(filterPin, capturePin, nullptr);
	if (FAILED(hr)) {
		WarningHR(L"HDevice::ConnectPins: failed to connect pins",
				hr);
		return false;
	}

	return true;
}

bool HDevice::RenderFilters(const GUID &category, const GUID &type,
		IBaseFilter *filter, IBaseFilter *capture)
{
	HRESULT hr;

	if (!EnsureInitialized(L"HDevice::RenderFilters") ||
	    !EnsureInactive(L"HDevice::RenderFilters"))
		return false;

	hr = builder->RenderStream(&category, &type, filter, NULL, capture);
	if (FAILED(hr)) {
		WarningHR(L"HDevice::ConnectFilters: RenderStream failed", hr);
		return false;
	}

	return true;
}

void HDevice::SetAudioBuffering(int bufferingMs)
{
	ComPtr<IPin> pin;
	bool success = GetFilterPin(audioFilter, MEDIATYPE_Audio,
			PIN_CATEGORY_CAPTURE, PINDIR_OUTPUT, &pin);
	if (!success)
		return;

	ComQIPtr<IAMStreamConfig> config(pin);
	if (!config)
		return;

	ComQIPtr<IAMBufferNegotiation> neg(pin);
	if (!neg)
		return;

	MediaTypePtr mt;
	if (FAILED(config->GetFormat(&mt)))
		return;

	if (mt->formattype != FORMAT_WaveFormatEx)
		return;
	if (mt->cbFormat != sizeof(WAVEFORMATEX))
		return;

	WAVEFORMATEX *wfex = (WAVEFORMATEX*)mt->pbFormat;

	ALLOCATOR_PROPERTIES props;
	props.cBuffers = -1;
	props.cbBuffer = wfex->nAvgBytesPerSec * bufferingMs / 1000;
	props.cbAlign = -1;
	props.cbPrefix = -1;
	HRESULT hr = neg->SuggestAllocatorProperties(&props);
	if (FAILED(hr))
		WarningHR(L"Could not set allocator properties on audio "
		          L"capture pin", hr);
}

bool HDevice::ConnectFilters()
{
	bool success = true;

	if (!EnsureInitialized(L"ConnectFilters") ||
	    !EnsureInactive(L"ConnectFilters"))
		return false;

	if (videoCapture != NULL) {
		success = ConnectPins(PIN_CATEGORY_CAPTURE,
				MEDIATYPE_Video, videoFilter,
				videoCapture);
		if (!success) {
			success = RenderFilters(PIN_CATEGORY_CAPTURE,
					MEDIATYPE_Video, videoFilter,
					videoCapture);
		}
	}

	if ((audioCapture || audioOutput) && success) {
		IBaseFilter *filter = (audioCapture != nullptr) ?
			audioCapture.Get() : audioOutput.Get();

		/* Stream engine has a bug where it will break if you try to
		 * set different audio buffering, so don't use audio buffering
		 * if using the stream engine's audio */
		bool streamEngine = audioConfig.useVideoDevice &&
			(videoConfig.name.find(L"Stream Engine") !=
			 std::string::npos);

		if (!streamEngine && audioCapture != nullptr)
			SetAudioBuffering(10);

		success = ConnectPins(PIN_CATEGORY_CAPTURE,
				MEDIATYPE_Audio, audioFilter,
				filter);
		if (!success) {
			success = RenderFilters(PIN_CATEGORY_CAPTURE,
					MEDIATYPE_Audio, audioFilter,
					filter);
		}
	}

	if (success)
		LogFilters(graph);

	return success;
}

void HDevice::DisconnectFilters()
{
	ComPtr<IEnumFilters>  filterEnum;
	HRESULT               hr;

	if (!graph)
		return;

	hr = graph->EnumFilters(&filterEnum);
	if (FAILED(hr))
		return;

	ComPtr<IBaseFilter> filter;
	while (filterEnum->Next(1, &filter, nullptr) == S_OK) {
		graph->RemoveFilter(filter);
		filterEnum->Reset();
	}
}

Result HDevice::Start()
{
	HRESULT hr;

	if (!EnsureInitialized(L"Start") ||
	    !EnsureInactive(L"Start"))
		return Result::Error;

	if (!!rocketEncoder)
		Sleep(ROCKET_WAIT_TIME_MS);

	hr = control->Run();

	if (FAILED(hr)) {
		if (hr == (HRESULT)0x8007001F) {
			WarningHR(L"Run failed, device already in use", hr);
			return Result::InUse;
		} else {
			WarningHR(L"Run failed", hr);
			return Result::Error;
		}
	}

	active = true;
	return Result::Success;
}

void HDevice::Stop()
{
	if (active) {
		control->Stop();
		active = false;
	}
}

}; /* namespace DShow */
