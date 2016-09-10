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

#include <vector>

using namespace std;

namespace DShow {

bool GetClosestVideoMediaType(IBaseFilter *filter, VideoConfig &config,
		MediaType &mt);
bool GetClosestAudioMediaType(IBaseFilter *filter, AudioConfig &config,
		MediaType &mt);

bool EnumVideoCaps(IPin *pin, vector<VideoInfo> &caps);
bool EnumAudioCaps(IPin *pin, vector<AudioInfo> &caps);

typedef bool (*EnumDeviceCallback)(void *param,
		IBaseFilter *filter,
		const wchar_t *deviceName,
		const wchar_t *devicePath);

bool EnumDevices(const GUID &type, EnumDeviceCallback callback, void *param);

}; /* namespace DShow */
