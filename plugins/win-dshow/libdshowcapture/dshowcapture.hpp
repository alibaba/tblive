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

#include <vector>
#include <string>
#include <functional>

#ifdef DSHOWCAPTURE_EXPORTS
	#define DSHOWCAPTURE_EXPORT __declspec(dllexport)
#else
	#define DSHOWCAPTURE_EXPORT
#endif

#define DSHOWCAPTURE_VERSION_MAJOR 0
#define DSHOWCAPTURE_VERSION_MINOR 5
#define DSHOWCAPTURE_VERSION_PATCH 8

#define MAKE_DSHOWCAPTURE_VERSION(major, minor, patch) \
		( (major << 24) | \
		  (minor << 16) | \
		  (patch)       )

#define DSHOWCAPTURE_VERSION MAKE_DSHOWCAPTURE_VERSION( \
		DSHOWCAPTURE_VERSION_MAJOR, \
		DSHOWCAPTURE_VERSION_MINOR, \
		DSHOWCAPTURE_VERSION_PATCH)

#define DSHOW_MAX_PLANES 8

namespace DShow {
	/* internal forward */
	struct HDevice;
	struct HVideoEncoder;
	struct VideoConfig;
	struct AudioConfig;

	typedef std::function<
		void (const VideoConfig &config,
			unsigned char *data, size_t size,
			long long startTime, long long stopTime)
		> VideoProc;

	typedef std::function<
		void (const AudioConfig &config,
			unsigned char *data, size_t size,
			long long startTime, long long stopTime)
		> AudioProc;

	enum class InitGraph {
		False,
		True
	};

	/** DirectShow configuration dialog type */
	enum class DialogType {
		ConfigVideo,
		ConfigAudio,
		ConfigCrossbar,
		ConfigCrossbar2
	};

	enum class VideoFormat {
		Any,
		Unknown,

		/* raw formats */
		ARGB = 100,
		XRGB,

		/* planar YUV formats */
		I420 = 200,
		NV12,
		YV12,

		/* packed YUV formats */
		YVYU = 300,
		YUY2,
		UYVY,
		HDYC,

		/* encoded formats */
		MJPEG = 400,
		H264
	};

	enum class AudioFormat {
		Any,
		Unknown,

		/* raw formats */
		Wave16bit = 100,
		WaveFloat,

		/* encoded formats */
		AAC = 200,
		AC3,
		MPGA /* MPEG 1 */
	};

	enum class AudioMode {
		Capture,
		DirectSound,
		WaveOut
	};

	enum class Result {
		Success,
		InUse,
		Error
	};

	struct VideoInfo {
		int         minCX, minCY;
		int         maxCX, maxCY;
		int         granularityCX, granularityCY;
		long long   minInterval, maxInterval;
		VideoFormat format;
	};

	struct AudioInfo {
		int         minChannels, maxChannels;
		int         channelsGranularity;
		int         minSampleRate, maxSampleRate;
		int         sampleRateGranularity;
		AudioFormat format;
	};

	struct DeviceId {
		std::wstring name;
		std::wstring path;
	};

	struct VideoDevice : DeviceId {
		bool audioAttached = false;
		std::vector<VideoInfo> caps;
	};

	struct AudioDevice : DeviceId {
		std::vector<AudioInfo> caps;
	};

	struct Config : DeviceId {
		/** Use the device's desired default config */
		bool        useDefaultConfig = true;
	};

	struct VideoConfig : Config {
		VideoProc   callback;

		/** Desired width/height of video.  */
		int         cx = 0, cy = 0;

		/** Desired frame interval (in 100-nanosecond units) */
		long long   frameInterval = 0;

		/** Internal video format. */
		VideoFormat internalFormat = VideoFormat::Any;

		/** Desired video format. */
		VideoFormat format = VideoFormat::Any;
	};

	struct AudioConfig : Config {
		AudioProc   callback;

		/**
		 * Use the audio attached to the video device
		 *
		 * (name/path memeber variables will be ignored)
		 */
		bool        useVideoDevice = false;

		/** Desired sample rate */
		int         sampleRate = 0;

		/** Desired channels */
		int         channels = 0;

		/** Desired audio format */
		AudioFormat format = AudioFormat::Any;

		/** Audio playback mode */
		AudioMode   mode = AudioMode::Capture;
	};

	class DSHOWCAPTURE_EXPORT Device {
		HDevice *context;

	public:
		Device(InitGraph initialize = InitGraph::False);
		~Device();

		bool        Valid() const;

		bool        ResetGraph();
		void        ShutdownGraph();

		bool        SetVideoConfig(VideoConfig *config);
		bool        SetAudioConfig(AudioConfig *config);

		/**
		 * Connects all the configured filters together.
		 *
		 * Call SetVideoConfig and/or SetAudioConfig before using.
		 */
		bool        ConnectFilters();

		Result      Start();
		void        Stop();

		bool        GetVideoConfig(VideoConfig &config) const;
		bool        GetAudioConfig(AudioConfig &config) const;
		bool        GetVideoDeviceId(DeviceId &id) const;
		bool        GetAudioDeviceId(DeviceId &id) const;

		/**
		 * Opens a DirectShow dialog associated with this device
		 *
		 * @param  type  The dialog type
		 */
		void        OpenDialog(void *hwnd, DialogType type) const;

		static bool EnumVideoDevices(std::vector<VideoDevice> &devices);
		static bool EnumAudioDevices(std::vector<AudioDevice> &devices);
	};

	struct VideoEncoderConfig : DeviceId {
		int fpsNumerator;
		int fpsDenominator;
		int bitrate;
		int keyframeInterval;
		int cx;
		int cy;
	};

	struct EncoderPacket {
		unsigned char  *data;
		size_t         size;
		long long      pts;
		long long      dts;
	};

	class VideoEncoder {
		HVideoEncoder *context;

	public:
		VideoEncoder();
		~VideoEncoder();

		bool Valid() const;
		bool Active() const;

		bool ResetGraph();

		bool SetConfig(VideoEncoderConfig &config);
		bool GetConfig(VideoEncoderConfig &config) const;

		bool Encode(unsigned char *data[DSHOW_MAX_PLANES],
				size_t linesize[DSHOW_MAX_PLANES],
				long long timestampStart,
				long long timestampEnd,
				EncoderPacket &packet,
				bool &new_packet);

		static bool EnumEncoders(std::vector<DeviceId> &encoders);
	};

	enum class LogType {
		Error,
		Warning,
		Info,
		Debug
	};

	typedef void (*LogCallback)(LogType type, const wchar_t *msg,
			void *param);

	DSHOWCAPTURE_EXPORT void SetLogCallback(LogCallback callback,
			void *param);
};
