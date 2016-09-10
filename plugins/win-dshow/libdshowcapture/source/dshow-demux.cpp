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

#include "dshow-demux.hpp"
#include "dshow-formats.hpp"
#include "log.hpp"

namespace DShow {

static inline DWORD VideoFormatToFourCC(VideoFormat format)
{
	if (format == VideoFormat::H264)
		return MAKEFOURCC('H', '2', '6', '4');

	return 0;
}

static inline const GUID &VideoFormatToSubType(VideoFormat format)
{
	if (format == VideoFormat::H264)
		return MEDIASUBTYPE_H264;

	return GUID_NULL;
}

bool CreateDemuxVideoPin(IBaseFilter *demuxFilter, MediaType &mt,
		long width, long height, long long frameTime,
		VideoFormat format)
{
	ComQIPtr<IMpeg2Demultiplexer> demuxer(demuxFilter);
	if (!demuxer) {
		Warning(L"CreateDemuxVideoPin: Failed to get "
		        L"IMpeg2Demultiplexer from filter");
		return false;
	}

	ComPtr<IPin>  pin;
	HRESULT       hr;

	VIDEOINFOHEADER *vih           = mt.AllocFormat<VIDEOINFOHEADER>();
	vih->bmiHeader.biSize          = sizeof(vih->bmiHeader);
	vih->bmiHeader.biWidth         = width;
	vih->bmiHeader.biHeight        = height;
	vih->bmiHeader.biCompression   = VideoFormatToFourCC(format);
	vih->rcSource.right            = width;
	vih->rcSource.bottom           = height;
	vih->AvgTimePerFrame           = frameTime;

	if (!vih->bmiHeader.biCompression) {
		Warning(L"CreateDemuxVideoPin: Invalid video format");
		return false;
	}

	mt->majortype            = MEDIATYPE_Video;
	mt->subtype              = VideoFormatToSubType(format);
	mt->formattype           = FORMAT_VideoInfo;
	mt->bTemporalCompression = true;

	wchar_t *name = (wchar_t*)CoTaskMemAlloc(sizeof(DEMUX_VIDEO_PIN));
	memcpy(name, DEMUX_VIDEO_PIN, sizeof(DEMUX_VIDEO_PIN));

	hr = demuxer->CreateOutputPin(mt, name, &pin);
	if (FAILED(hr)) {
		WarningHR(L"CreateDemuxVideoPin: Failed to create video pin "
		          L"on demuxer", hr);
		return false;
	}

	return true;
}

static inline WORD AudioFormatToFormatTag(AudioFormat format)
{
	if (format == AudioFormat::AAC)
		return WAVE_FORMAT_RAW_AAC1;
	else if (format == AudioFormat::AC3)
		return WAVE_FORMAT_DVM;
	else if (format == AudioFormat::MPGA)
		return WAVE_FORMAT_MPEG;

	return 0;
}

static inline const GUID &AudioFormatToSubType(AudioFormat format)
{
	if (format == AudioFormat::AAC)
		return MEDIASUBTYPE_RAW_AAC1;
	else if (format == AudioFormat::AC3)
		return MEDIASUBTYPE_DVM;
	else if (format == AudioFormat::MPGA)
		return MEDIASUBTYPE_MPEG1AudioPayload;

	return GUID_NULL;
}

bool CreateDemuxAudioPin(IBaseFilter *demuxFilter, MediaType &mt,
		DWORD samplesPerSec, WORD bitsPerSample, WORD channels,
		AudioFormat format)
{
	ComQIPtr<IMpeg2Demultiplexer> demuxer(demuxFilter);
	if (!demuxer) {
		Warning(L"CreateDemuxAudioPin: Failed to get "
		        L"IMpeg2Demultiplexer from filter");
		return false;
	}

	ComPtr<IPin>  pin;
	HRESULT       hr;

	WAVEFORMATEX *wfex   = mt.AllocFormat<WAVEFORMATEX>();
	wfex->wFormatTag     = AudioFormatToFormatTag(format);
	wfex->nChannels      = channels;
	wfex->nSamplesPerSec = samplesPerSec;
	wfex->wBitsPerSample = bitsPerSample;

	if (!wfex->wFormatTag) {
		Warning(L"CreateDemuxAudioPin: Invalid audio format");
		return false;
	}

	mt->majortype            = MEDIATYPE_Audio;
	mt->subtype              = AudioFormatToSubType(format);
	mt->formattype           = FORMAT_WaveFormatEx;
	mt->bTemporalCompression = true;

	wchar_t *name = (wchar_t*)CoTaskMemAlloc(sizeof(DEMUX_AUDIO_PIN));
	memcpy(name, DEMUX_AUDIO_PIN, sizeof(DEMUX_AUDIO_PIN));

	hr = demuxer->CreateOutputPin(mt, name, &pin);
	if (FAILED(hr)) {
		WarningHR(L"CreateDemuxAudioPin: Failed to create audio pin "
		          L"on demuxer", hr);
		return false;
	}

	return true;
}

}; /* namespace DShow */
