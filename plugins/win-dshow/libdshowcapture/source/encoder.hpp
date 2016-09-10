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
#include "output-filter.hpp"
#include "capture-filter.hpp"

#include <string>
#include <vector>
#include <deque>
#include <mutex>
using namespace std;

struct EncodedData {
	vector<unsigned char>          data;

	inline EncodedData() {}

	inline EncodedData(unsigned char *data_, size_t size)
	{
		data.resize(size);
		memcpy(data.data(), data_, size);
	}
};

namespace DShow {

struct HVideoEncoder {
	ComPtr<IGraphBuilder>          graph;
	ComPtr<ICaptureGraphBuilder2>  builder;
	ComPtr<IMediaControl>          control;

	ComPtr<IBaseFilter>            encoder;
	ComPtr<IBaseFilter>            device;
	ComPtr<OutputFilter>           output;
	ComPtr<CaptureFilter>          capture;

	VideoEncoderConfig             config;

	mutex                          packetMutex;
	deque<EncodedData>             packets;
	EncodedData                    curPacket;

	deque<long long>               ptsVals;

	bool                           initialized = false;
	bool                           active = false;

	HVideoEncoder();
	~HVideoEncoder();

	bool SetupCrossbar();

	void Receive(IMediaSample *s);

	bool ConnectFilters();

	void InitializeVideoFormat(MediaType &mt);
	bool SetupEncoder(IBaseFilter *filter);

	bool SetConfig(VideoEncoderConfig &config);

	bool Encode(unsigned char *frame[DSHOW_MAX_PLANES],
			size_t linesize[DSHOW_MAX_PLANES],
			long long timestampStart, long long timestampEnd,
			EncoderPacket &packet, bool &new_packet);
};

};
