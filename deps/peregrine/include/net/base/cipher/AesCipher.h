#ifndef __NET_BASE_AES_H__
#define __NET_BASE_AES_H__

#include "net/base/net_export.h"
#include <string>


NET_EXPORT std::string AesEncode(const std::string& key, const std::string& src);
NET_EXPORT std::string AesDecode(const std::string& key, const std::string& src, bool bPadding = false);

#endif // __NET_BASE_AES_H__
