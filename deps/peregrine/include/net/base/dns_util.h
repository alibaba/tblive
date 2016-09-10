// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_BASE_DNS_UTIL_H_
#define NET_BASE_DNS_UTIL_H_

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/strings/string_piece.h"
#include "net/base/net_export.h"

namespace net {

// DNSDomainFromDot - convert a domain string to DNS format. From DJB's
// public domain DNS library.
//
//   dotted: a string in dotted form: "www.google.com"
//   out: a result in DNS form: "\x03www\x06google\x03com\x00"
NET_EXPORT_PRIVATE bool DNSDomainFromDot(const base::StringPiece& dotted,
                                         std::string* out);

// DNSDomainToString converts a domain in DNS format to a dotted string.
// Excludes the dot at the end.
NET_EXPORT_PRIVATE std::string DNSDomainToString(
    const base::StringPiece& domain);

// Returns true iff the given character is in the set of valid DNS label
// characters as given in RFC 3490, 4.1, 3(a)
NET_EXPORT_PRIVATE bool IsSTD3ASCIIValidCharacter(char c);

// Returns the hostname by trimming the ending dot, if one exists.
NET_EXPORT std::string TrimEndingDot(const base::StringPiece& host);

// Convert domain to ip list
NET_EXPORT void DomainToIpList(const std::string& domain, std::vector<std::string> ipVec);

}  // namespace net

#endif  // NET_BASE_DNS_UTIL_H_
