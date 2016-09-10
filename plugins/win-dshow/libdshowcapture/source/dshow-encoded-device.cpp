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
#include "dshow-media-type.hpp"
#include "dshow-formats.hpp"
#include "dshow-demux.hpp"
#include "capture-filter.hpp"
#include "device.hpp"
#include "log.hpp"

namespace DShow {

static inline bool CreateFilters(IBaseFilter *filter,
		IBaseFilter **crossbar, IBaseFilter **encoder,
		IBaseFilter **demuxer)
{
	ComPtr<IPin>  inputPin;
	ComPtr<IPin>  outputPin;
	REGPINMEDIUM  inMedium;
	REGPINMEDIUM  outMedium;
	bool          hasOutMedium;
	HRESULT       hr;

	if (!GetPinByName(filter, PINDIR_INPUT, nullptr, &inputPin)) {
		Warning(L"Encoded Device: Failed to get input pin");
		return false;
	}

	if (!GetPinByName(filter, PINDIR_OUTPUT, nullptr, &outputPin)) {
		Warning(L"Encoded Device: Failed to get output pin");
		return false;
	}

	if (!GetPinMedium(inputPin, inMedium)) {
		Warning(L"Encoded Device: Failed to get input pin medium");
		return false;
	}

	hasOutMedium = GetPinMedium(outputPin, outMedium);

	if (!GetFilterByMedium(AM_KSCATEGORY_CROSSBAR, inMedium, crossbar)) {
		Warning(L"Encoded Device: Failed to get crossbar filter");
		return false;
	}

	/* perfectly okay if there's no encoder filter, some don't have them */
	if (hasOutMedium)
		GetFilterByMedium(KSCATEGORY_ENCODER, outMedium, encoder);

	hr = CoCreateInstance(CLSID_MPEG2Demultiplexer, nullptr,
			CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)demuxer);
	if (FAILED(hr)) {
		WarningHR(L"Encoded Device: Failed to create demuxer", hr);
		return false;
	}

	return true;
}

static inline bool ConnectEncodedFilters(IGraphBuilder *graph,
		IBaseFilter *filter, IBaseFilter *crossbar,
		IBaseFilter *encoder, IBaseFilter *demuxer)
{
	if (!DirectConnectFilters(graph, crossbar, filter)) {
		Warning(L"Encoded Device: Failed to connect crossbar to "
		        L"device");
		return false;
	}

	if (!!encoder) {
		if (!DirectConnectFilters(graph, filter, encoder)) {
			Warning(L"Encoded Device: Failed to connect device to "
			        L"encoder");
			return false;
		}

		if (!DirectConnectFilters(graph, encoder, demuxer)) {
			Warning(L"Encoded Device: Failed to connect encoder to "
				L"demuxer");
			return false;
		}
	} else {
		if (!DirectConnectFilters(graph, filter, demuxer)) {
			Warning(L"Encoded Device: Failed to connect device to "
			        L"demuxer");
			return false;
		}
	}

	return true;
}

static inline bool MapPacketIDs(IBaseFilter *demuxer, ULONG video, ULONG audio)
{
	ComPtr<IPin>  videoPin, audioPin;
	HRESULT       hr;

	if (!GetPinByName(demuxer, PINDIR_OUTPUT, DEMUX_VIDEO_PIN, &videoPin)) {
		Warning(L"Encoded Device: Could not get video pin from "
		        L"demuxer");
		return false;
	}

	if (!GetPinByName(demuxer, PINDIR_OUTPUT, DEMUX_AUDIO_PIN, &audioPin)) {
		Warning(L"Encoded Device: Could not get audio pin from "
		        L"demuxer");
		return false;
	}

	hr = MapPinToPacketID(videoPin, video);
	if (FAILED(hr)) {
		WarningHR(L"Encoded Device: Failed to map demuxer video pin "
		          L"packet ID", hr);
		return false;
	}

	hr = MapPinToPacketID(audioPin, audio);
	if (FAILED(hr)) {
		WarningHR(L"Encoded Device: Failed to map demuxer audio pin "
		          L"packet ID", hr);
		return false;
	}

	return true;
}

/*
 * rocket-specific workaround code.  I have no idea what any of these numbers
 * are except the GUID which was obvious.  All I know is calling
 * IKsPropertySet::Set turns on/off some sort of 'mode' on the device.
 * I discovered this merely by chance while monitoring API usage in other
 * programs because I could not figure out how the hell to get this thing
 * to turn on.
 */
static const GUID RocketEncoderGUID =
{0x99100000, 0xa330, 0x11e1, {0xa3, 0x80, 0x99, 0x10, 0x68, 0x64, 0x00, 0x00}};

struct RocketPropStruct {
	DWORD dwSize;
	DWORD unknown1;
	DWORD unknown2;
	DWORD unknown3;
	DWORD code;
	DWORD unknown4;
	BOOL  enabled;
};

struct RocketInstStruct {
	DWORD code;
	DWORD unknown1;
};

bool SetRocketEnabled(IBaseFilter *encoder, bool enable)
{
	static const ULONG rocketEnableId   = 0x9910E001;
	static const DWORD rocketEnableCode = 0x38384001;
	RocketInstStruct   rocketInstance   = {};
	RocketPropStruct   rocketProperty   = {};

	ComQIPtr<IKsPropertySet> propertySet(encoder);
	if (!propertySet)
		return false;

	rocketProperty.dwSize  = sizeof(rocketProperty);
	rocketProperty.code    = rocketEnableCode;
	rocketProperty.enabled = enable;
	rocketInstance.code    = rocketEnableCode;

	HRESULT hr = propertySet->Set(RocketEncoderGUID, rocketEnableId,
			&rocketInstance, sizeof(rocketInstance),
			&rocketProperty, sizeof(rocketProperty));

	return SUCCEEDED(hr);
}

bool HDevice::SetupEncodedVideoCapture(IBaseFilter *filter,
			VideoConfig &config,
			const EncodedDevice &info)
{
	ComPtr<IBaseFilter>  crossbar;
	ComPtr<IBaseFilter>  encoder;
	ComPtr<IBaseFilter>  demuxer;
	MediaType            mtVideo;
	MediaType            mtAudio;

	if (!CreateFilters(filter, &crossbar, &encoder, &demuxer))
		return false;

	if (!CreateDemuxVideoPin(demuxer, mtVideo, info.width, info.height,
				info.frameInterval, info.videoFormat))
		return false;

	if (!CreateDemuxAudioPin(demuxer, mtAudio, info.samplesPerSec,
				16, 2, info.audioFormat))
		return false;

	config.cx             = info.width;
	config.cy             = info.height;
	config.frameInterval  = info.frameInterval;
	config.format         = info.videoFormat;
	config.internalFormat = info.videoFormat;

	PinCaptureInfo pci;
	pci.callback          = [this] (IMediaSample *s) {Receive(true, s);};
	pci.expectedMajorType = mtVideo->majortype;
	pci.expectedSubType   = mtVideo->subtype;

	videoCapture = new CaptureFilter(pci);
	videoFilter  = demuxer;

	if (!!encoder && config.name.find(L"IT9910") != std::string::npos) {
		rocketEncoder = encoder;

		if (!SetRocketEnabled(rocketEncoder, true))
			return false;
	}

	graph->AddFilter(crossbar,     L"Crossbar");
	graph->AddFilter(filter,       L"Device");
	graph->AddFilter(demuxer,      L"Demuxer");
	graph->AddFilter(videoCapture, L"Capture Filter");

	if (!!encoder)
		graph->AddFilter(encoder, L"Encoder");

	bool success = ConnectEncodedFilters(graph, filter, crossbar,
			encoder, demuxer);
	if (success)
		success = MapPacketIDs(demuxer, info.videoPacketID,
				info.audioPacketID);

	encodedDevice = success;
	return success;
}

}; /* namespace DShow */
