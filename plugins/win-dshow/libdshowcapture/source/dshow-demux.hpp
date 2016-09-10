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

#include "../dshowcapture.hpp"
#include "dshow-base.hpp"
#include "dshow-media-type.hpp"

namespace DShow {

#define DEMUX_VIDEO_PIN L"Demuxer Video Pin"
#define DEMUX_AUDIO_PIN L"Demuxer Audio Pin"

bool CreateDemuxVideoPin(IBaseFilter *demuxFilter, MediaType &mt,
		long width, long height, long long frameTime,
		VideoFormat format);

bool CreateDemuxAudioPin(IBaseFilter *demuxFilter, MediaType &mt,
		DWORD samplesPerSec, WORD bitsPerSample, WORD channels,
		AudioFormat format);

}; /* namespace DShow */
