#pragma once

#ifndef NET_BASE_ERROR_TRANSLATE
#define NET_BASE_ERROR_TRANSLATE

#include "net/base/net_export.h"
#include <string>


namespace net {

// GetErrorMsg
NET_EXPORT std::wstring GetErrorMsg( int code );


}// namespace net

#endif //NET_BASE_ERROR_TRANSLATE
