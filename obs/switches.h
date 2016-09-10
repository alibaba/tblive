#pragma once

namespace switches {

// ---- Command line ----

// transfer "tblive://xxx" shell command
extern const char kTbliveCmd[];

// set disable login window mode
extern const char kDisableLogin[];

// set rtmp url
extern const char kRtmpUrl[];


// ---- Shell cmd and arguments ----

extern const wchar_t cmdStartLive[];
extern const wchar_t argLoginUser[];
extern const wchar_t argTryAutoLogin[];


extern const wchar_t cmdSetFocus[];


}  // namespace switches
