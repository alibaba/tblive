#ifndef      _NET_BASE_RSA_WRAPPER_H_
#define      _NET_BASE_RSA_WRAPPER_H_

#include "net/base/net_export.h"

#include <string>

NET_EXPORT int RSAPublicEncrypt(const char* szPubKey,
							   const int nPubKey, 
							   const char* szOri,
							   const int nOriLen,
							   std::string& strResult);
#endif
