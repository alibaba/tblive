// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_HTTP_HTTP_SERVER_PROPERTIES_H_
#define NET_HTTP_HTTP_SERVER_PROPERTIES_H_

#include <map>
#include <string>
#include "base/basictypes.h"
#include "base/memory/weak_ptr.h"
#include "net/base/host_port_pair.h"
#include "net/base/net_export.h"
#include "net/http/http_pipelined_host_capability.h"
#include "net/socket/next_proto.h"
#include "net/spdy/spdy_framer.h"  // TODO(willchan): Reconsider this.

namespace net {

enum AlternateProtocol {
  NPN_SPDY_1 = 0,
  NPN_SPDY_MINIMUM_VERSION = NPN_SPDY_1,
  NPN_SPDY_2,
  NPN_SPDY_3,
  NPN_SPDY_3_1,
  NPN_SPDY_4A2,
  // We lump in HTTP/2 with the SPDY protocols for now.
  NPN_HTTP2_DRAFT_04,
  NPN_SPDY_MAXIMUM_VERSION = NPN_HTTP2_DRAFT_04,
  QUIC,
  NUM_ALTERNATE_PROTOCOLS,
  ALTERNATE_PROTOCOL_BROKEN,  // The alternate protocol is known to be broken.
  UNINITIALIZED_ALTERNATE_PROTOCOL,
};

NET_EXPORT const char* AlternateProtocolToString(AlternateProtocol protocol);
NET_EXPORT AlternateProtocol AlternateProtocolFromString(
    const std::string& protocol);
NET_EXPORT_PRIVATE AlternateProtocol AlternateProtocolFromNextProto(
    NextProto next_proto);

struct NET_EXPORT PortAlternateProtocolPair {
  bool Equals(const PortAlternateProtocolPair& other) const {
    return port == other.port && protocol == other.protocol;
  }

  std::string ToString() const;

  uint16 port;
  AlternateProtocol protocol;
};

typedef std::map<HostPortPair, PortAlternateProtocolPair> AlternateProtocolMap;
typedef std::map<HostPortPair, SettingsMap> SpdySettingsMap;
typedef std::map<HostPortPair,
        HttpPipelinedHostCapability> PipelineCapabilityMap;

extern const char kAlternateProtocolHeader[];

// The interface for setting/retrieving the HTTP server properties.
// Currently, this class manages servers':
// * SPDY support (based on NPN results)
// * Alternate-Protocol support
// * Spdy Settings (like CWND ID field)
class NET_EXPORT HttpServerProperties {
 public:
  HttpServerProperties() {}
  virtual ~HttpServerProperties() {}

  // Gets a weak pointer for this object.
  virtual base::WeakPtr<HttpServerProperties> GetWeakPtr() = 0;

  // Deletes all data.
  virtual void Clear() = 0;

  // Returns true if |server| supports SPDY.
  virtual bool SupportsSpdy(const HostPortPair& server) const = 0;

  // Add |server| into the persistent store. Should only be called from IO
  // thread.
  virtual void SetSupportsSpdy(const HostPortPair& server,
                               bool support_spdy) = 0;

  // Returns true if |server| has an Alternate-Protocol header.
  virtual bool HasAlternateProtocol(const HostPortPair& server) const = 0;

  // Returns the Alternate-Protocol and port for |server|.
  // HasAlternateProtocol(server) must be true.
  virtual PortAlternateProtocolPair GetAlternateProtocol(
      const HostPortPair& server) const = 0;

  // Sets the Alternate-Protocol for |server|.
  virtual void SetAlternateProtocol(const HostPortPair& server,
                                    uint16 alternate_port,
                                    AlternateProtocol alternate_protocol) = 0;

  // Sets the Alternate-Protocol for |server| to be BROKEN.
  virtual void SetBrokenAlternateProtocol(const HostPortPair& server) = 0;

  // Returns all Alternate-Protocol mappings.
  virtual const AlternateProtocolMap& alternate_protocol_map() const = 0;

  // Gets a reference to the SettingsMap stored for a host.
  // If no settings are stored, returns an empty SettingsMap.
  virtual const SettingsMap& GetSpdySettings(
      const HostPortPair& host_port_pair) const = 0;

  // Saves an individual SPDY setting for a host. Returns true if SPDY setting
  // is to be persisted.
  virtual bool SetSpdySetting(const HostPortPair& host_port_pair,
                              SpdySettingsIds id,
                              SpdySettingsFlags flags,
                              uint32 value) = 0;

  // Clears all SPDY settings for a host.
  virtual void ClearSpdySettings(const HostPortPair& host_port_pair) = 0;

  // Clears all SPDY settings for all hosts.
  virtual void ClearAllSpdySettings() = 0;

  // Returns all persistent SPDY settings.
  virtual const SpdySettingsMap& spdy_settings_map() const = 0;

  virtual HttpPipelinedHostCapability GetPipelineCapability(
      const HostPortPair& origin) = 0;

  virtual void SetPipelineCapability(
      const HostPortPair& origin,
      HttpPipelinedHostCapability capability) = 0;

  virtual void ClearPipelineCapabilities() = 0;

  virtual PipelineCapabilityMap GetPipelineCapabilityMap() const = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(HttpServerProperties);
};

}  // namespace net

#endif  // NET_HTTP_HTTP_SERVER_PROPERTIES_H_
