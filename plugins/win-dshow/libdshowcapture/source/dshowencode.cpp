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

#include "../dshowcapture.hpp"
#include "dshow-base.hpp"
#include "dshow-enum.hpp"
#include "encoder.hpp"
#include "log.hpp"

namespace DShow {

VideoEncoder::VideoEncoder() : context(new HVideoEncoder)
{
}

VideoEncoder::~VideoEncoder()
{
	delete context;
}

bool VideoEncoder::Valid() const
{
	return context->initialized;
}

bool VideoEncoder::Active() const
{
	return context->active;
}

bool VideoEncoder::ResetGraph()
{
	delete context;
	context = new HVideoEncoder;

	return context->initialized;
}

bool VideoEncoder::SetConfig(VideoEncoderConfig &config)
{
	if (context->active) {
		delete context;
		context = new HVideoEncoder;
	}

	return context->SetConfig(config);
}

bool VideoEncoder::GetConfig(VideoEncoderConfig &config) const
{
	if (context->encoder == nullptr)
		return false;

	config = context->config;
	return true;
}

bool VideoEncoder::Encode(unsigned char *data[DSHOW_MAX_PLANES],
		size_t linesize[DSHOW_MAX_PLANES],
		long long timestampStart, long long timestampEnd,
		EncoderPacket &packet, bool &new_packet)
{
	if (context->encoder == nullptr)
		return false;

	return context->Encode(data, linesize, timestampStart, timestampEnd,
			packet, new_packet);
}

static bool EnumVideoEncoder(vector<DeviceId> &encoders,
		IBaseFilter *encoder,
		const wchar_t *deviceName,
		const wchar_t *devicePath)
{
	DeviceId id;
	bool validDevice =
		wcsstr(deviceName, L"C985") ||
		wcsstr(deviceName, L"C353");

	if (!validDevice)
		return true;

	id.name = deviceName;
	id.path = devicePath;
	encoders.push_back(id);

	(void)encoder;
	return true;
}

bool VideoEncoder::EnumEncoders(vector<DeviceId> &encoders)
{
	encoders.clear();
	return EnumDevices(KSCATEGORY_ENCODER,
			EnumDeviceCallback(EnumVideoEncoder), &encoders);
}

};
