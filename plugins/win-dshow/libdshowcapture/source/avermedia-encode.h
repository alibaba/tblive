/*
 * aver_prophwencode.h  --  This header filer is provide to 3rd party AP
 * to use the HW encode function of AVerMedia.
 *
 * Copyright (C) 2014 AVerMedia TECHNOLOGIES, Inc.
 *
 * Authors: Morris Pan, AVerMedia TECHNOLOGIES, Inc. <morris.pan@avermedia.com>
 *
 * This content is released under the MIT License
 * (http://opensource.org/licenses/MIT).
 *
 */
#pragma once

static const GUID AVER_HW_ENCODE_PROPERTY =
{0x1bd55918, 0xbaf5, 0x4781, {0x8d, 0x76, 0xe0, 0xa0, 0xa5, 0xe1, 0xd2, 0xb8}};

enum {
	// @brief	PropertySet Enumeration
	// param	AVER_PARAMETERS          
	PROPERTY_HW_ENCODE_PARAMETER           = 0
};

enum {
	// property to set/get the encode frame rate
	// ulParam1 = Frames per second
	AVER_PARAMETER_ENCODE_FRAME_RATE       = 0,

	// property to set/get the encode bit rate
	// ulParam1 = Bitrate (kb/s)
	AVER_PARAMETER_ENCODE_BIT_RATE         = 1,

	// property to get the output resolution
	// ulParam1 = Resolution width
	// ulParam2 = Resolution height
	AVER_PARAMETER_CURRENT_RESOLUTION      = 2,

	// property to set the output resolution
	// ulParam1 = Resolution width
	// ulParam2 = Resolution height
	AVER_PARAMETER_ENCODE_RESOLUTION       = 3,

	// property to set/get the encode GOP
	// ulParam1 = GOP length
	AVER_PARAMETER_ENCODE_GOP              = 4,

	// property to insert an I frame to the encoded stream
	AVER_PARAMETER_INSERT_I_FRAME          = 6
};

struct AVER_PARAMETERS {
	// @brief   Use the PROPERTY_PARAMETER Property to Get or Set the
	//          Device Parameter.
	//
	// param    ulIndex   Parameter Index (AVER_PARAMETER_*)
	// param    ulParam1  Parameter 1 (if any)
	// param    ulParam2  Parameter 2 (if any)
	// param    ulParam3  Parameter 3 (if any)
	ULONG ulIndex;
	ULONG ulParam1;
	ULONG ulParam2;
	ULONG ulParam3;
};
