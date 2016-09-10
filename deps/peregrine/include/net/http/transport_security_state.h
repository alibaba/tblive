// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_HTTP_TRANSPORT_SECURITY_STATE_H_
#define NET_HTTP_TRANSPORT_SECURITY_STATE_H_

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "base/basictypes.h"
#include "base/gtest_prod_util.h"
#include "base/threading/non_thread_safe.h"
#include "base/time/time.h"
#include "net/base/net_export.h"
#include "net/cert/x509_cert_types.h"
#include "net/cert/x509_certificate.h"

namespace net {

class SSLInfo;

// Tracks which hosts have enabled strict transport security and/or public
// key pins.
//
// This object manages the in-memory store. Register a Delegate with
// |SetDelegate| to persist the state to disk.
//
// HTTP strict transport security (HSTS) is defined in
// http://tools.ietf.org/html/ietf-websec-strict-transport-sec, and
// HTTP-based dynamic public key pinning (HPKP) is defined in
// http://tools.ietf.org/html/ietf-websec-key-pinning.
class NET_EXPORT TransportSecurityState
    : NON_EXPORTED_BASE(public base::NonThreadSafe) {
 public:
  class Delegate {
   public:
    // This function may not block and may be called with internal locks held.
    // Thus it must not reenter the TransportSecurityState object.
    virtual void StateIsDirty(TransportSecurityState* state) = 0;

   protected:
    virtual ~Delegate() {}
  };

  TransportSecurityState();
  ~TransportSecurityState();

  // A DomainState describes the transport security state (required upgrade
  // to HTTPS, and/or any public key pins).
  class NET_EXPORT DomainState {
   public:
    enum UpgradeMode {
      // These numbers must match those in hsts_view.js, function modeToString.
      MODE_FORCE_HTTPS = 0,
      MODE_DEFAULT = 1,
    };

    DomainState();
    ~DomainState();

    // Takes a set of SubjectPublicKeyInfo |hashes| and returns true if:
    //   1) |bad_static_spki_hashes| does not intersect |hashes|; AND
    //   2) Both |static_spki_hashes| and |dynamic_spki_hashes| are empty
    //      or at least one of them intersects |hashes|.
    //
    // |{dynamic,static}_spki_hashes| contain trustworthy public key hashes,
    // any one of which is sufficient to validate the certificate chain in
    // question. The public keys could be of a root CA, intermediate CA, or
    // leaf certificate, depending on the security vs. disaster recovery
    // tradeoff selected. (Pinning only to leaf certifiates increases
    // security because you no longer trust any CAs, but it hampers disaster
    // recovery because you can't just get a new certificate signed by the
    // CA.)
    //
    // |bad_static_spki_hashes| contains public keys that we don't want to
    // trust.
    bool CheckPublicKeyPins(const HashValueVector& hashes) const;

    // Returns true if any of the HashValueVectors |static_spki_hashes|,
    // |bad_static_spki_hashes|, or |dynamic_spki_hashes| contains any
    // items.
    bool HasPublicKeyPins() const;

    // ShouldUpgradeToSSL returns true iff, given the |mode| of this
    // DomainState, HTTP requests should be internally redirected to HTTPS
    // (also if the "ws" WebSocket request should be upgraded to "wss")
    bool ShouldUpgradeToSSL() const;

    // ShouldSSLErrorsBeFatal returns true iff HTTPS errors should cause
    // hard-fail behavior (e.g. if HSTS is set for the domain)
    bool ShouldSSLErrorsBeFatal() const;

    UpgradeMode upgrade_mode;

    // The absolute time (UTC) when this DomainState was first created.
    //
    // Static entries do not have a created time.
    base::Time created;

    // The absolute time (UTC) when the |upgrade_mode|, if set to
    // UPGRADE_ALWAYS, downgrades to UPGRADE_NEVER.
    base::Time upgrade_expiry;

    // Are subdomains subject to this DomainState, for the purposes of
    // upgrading to HTTPS?
    bool sts_include_subdomains;

    // Are subdomains subject to this DomainState, for the purposes of
    // Pin Validation?
    bool pkp_include_subdomains;

    // Optional; hashes of static pinned SubjectPublicKeyInfos. Unless both
    // are empty, at least one of |static_spki_hashes| and
    // |dynamic_spki_hashes| MUST intersect with the set of SPKIs in the TLS
    // server's certificate chain.
    //
    // |dynamic_spki_hashes| take precedence over |static_spki_hashes|.
    // That is, |IsChainOfPublicKeysPermitted| first checks dynamic pins and
    // then checks static pins.
    HashValueVector static_spki_hashes;

    // Optional; hashes of dynamically pinned SubjectPublicKeyInfos.
    HashValueVector dynamic_spki_hashes;

    // The absolute time (UTC) when the |dynamic_spki_hashes| expire.
    base::Time dynamic_spki_hashes_expiry;

    // Optional; hashes of static known-bad SubjectPublicKeyInfos which
    // MUST NOT intersect with the set of SPKIs in the TLS server's
    // certificate chain.
    HashValueVector bad_static_spki_hashes;

    // The following members are not valid when stored in |enabled_hosts_|:

    // The domain which matched during a search for this DomainState entry.
    // Updated by |GetDomainState|, |GetDynamicDomainState|, and
    // |GetStaticDomainState|.
    std::string domain;
  };

  class NET_EXPORT Iterator {
   public:
    explicit Iterator(const TransportSecurityState& state);
    ~Iterator();

    bool HasNext() const { return iterator_ != end_; }
    void Advance() { ++iterator_; }
    const std::string& hostname() const { return iterator_->first; }
    const DomainState& domain_state() const { return iterator_->second; }

   private:
    std::map<std::string, DomainState>::const_iterator iterator_;
    std::map<std::string, DomainState>::const_iterator end_;
  };

  // Assign a |Delegate| for persisting the transport security state. If
  // |NULL|, state will not be persisted. The caller retains
  // ownership of |delegate|.
  // Note: This is only used for serializing/deserializing the
  // TransportSecurityState.
  void SetDelegate(Delegate* delegate);

  // Clears all dynamic data (e.g. HSTS and HPKP data).
  //
  // Does NOT persist changes using the Delegate, as this function is only
  // used to clear any dynamic data prior to re-loading it from a file.
  // Note: This is only used for serializing/deserializing the
  // TransportSecurityState.
  void ClearDynamicData();

  // Inserts |state| into |enabled_hosts_| under the key |hashed_host|.
  // |hashed_host| is already in the internal representation
  // HashHost(CanonicalizeHost(host)).
  // Note: This is only used for serializing/deserializing the
  // TransportSecurityState.
  void AddOrUpdateEnabledHosts(const std::string& hashed_host,
                               const DomainState& state);

  // Deletes all dynamic data (e.g. HSTS or HPKP data) created since a given
  // time.
  //
  // If any entries are deleted, the new state will be persisted through
  // the Delegate (if any).
  void DeleteAllDynamicDataSince(const base::Time& time);

  // Deletes any dynamic data stored for |host| (e.g. HSTS or HPKP data).
  // If |host| doesn't have an exact entry then no action is taken. Does
  // not delete static (i.e. preloaded) data.  Returns true iff an entry
  // was deleted.
  //
  // If an entry is deleted, the new state will be persisted through
  // the Delegate (if any).
  bool DeleteDynamicDataForHost(const std::string& host);

  // Returns true and updates |*result| iff there is a DomainState for
  // |host|.
  //
  // If |sni_enabled| is true, searches the static pins defined for
  // SNI-using hosts as well as the rest of the pins.
  //
  // If |host| matches both an exact entry and is a subdomain of another
  // entry, the exact match determines the return value.
  //
  // Note that this method is not const because it opportunistically removes
  // entries that have expired.
  bool GetDomainState(const std::string& host,
                      bool sni_enabled,
                      DomainState* result);

  // Processes an HSTS header value from the host, adding entries to
  // dynamic state if necessary.
  bool AddHSTSHeader(const std::string& host, const std::string& value);

  // Processes an HPKP header value from the host, adding entries to
  // dynamic state if necessary.  ssl_info is used to check that
  // the specified pins overlap with the certificate chain.
  bool AddHPKPHeader(const std::string& host, const std::string& value,
                     const SSLInfo& ssl_info);

  // Adds explicitly-specified data as if it was processed from an
  // HSTS header (used for net-internals and unit tests).
  bool AddHSTS(const std::string& host, const base::Time& expiry,
               bool include_subdomains);

  // Adds explicitly-specified data as if it was processed from an
  // HPKP header (used for net-internals and unit tests).
  bool AddHPKP(const std::string& host, const base::Time& expiry,
               bool include_subdomains, const HashValueVector& hashes);

  // Returns true iff we have any static public key pins for the |host| and
  // iff its set of required pins is the set we expect for Google
  // properties.
  //
  // If |sni_enabled| is true, searches the static pins defined for
  // SNI-using hosts as well as the rest of the pins.
  //
  // If |host| matches both an exact entry and is a subdomain of another
  // entry, the exact match determines the return value.
  static bool IsGooglePinnedProperty(const std::string& host,
                                     bool sni_enabled);

  // The maximum number of seconds for which we'll cache an HSTS request.
  static const long int kMaxHSTSAgeSecs;

  // Send an UMA report on pin validation failure, if the host is in a
  // statically-defined list of domains.
  //
  // TODO(palmer): This doesn't really belong here, and should be moved into
  // the exactly one call site. This requires unifying |struct HSTSPreload|
  // (an implementation detail of this class) with a more generic
  // representation of first-class DomainStates, and exposing the preloads
  // to the caller with |GetStaticDomainState|.
  static void ReportUMAOnPinFailure(const std::string& host);

  // IsBuildTimely returns true if the current build is new enough ensure that
  // built in security information (i.e. HSTS preloading and pinning
  // information) is timely.
  static bool IsBuildTimely();

 private:
  friend class TransportSecurityStateTest;
  FRIEND_TEST_ALL_PREFIXES(HttpSecurityHeadersTest,
                           UpdateDynamicPKPOnly);

  typedef std::map<std::string, DomainState> DomainStateMap;

  // If a Delegate is present, notify it that the internal state has
  // changed.
  void DirtyNotify();

  // Enable TransportSecurity for |host|. |state| supercedes any previous
  // state for the |host|, including static entries.
  //
  // The new state for |host| is persisted using the Delegate (if any).
  void EnableHost(const std::string& host, const DomainState& state);

  // Converts |hostname| from dotted form ("www.google.com") to the form
  // used in DNS: "\x03www\x06google\x03com", lowercases that, and returns
  // the result.
  static std::string CanonicalizeHost(const std::string& hostname);

  // Returns true and updates |*result| iff there is a static DomainState for
  // |host|.
  //
  // |GetStaticDomainState| is identical to |GetDomainState| except that it
  // searches only the statically-defined transport security state, ignoring
  // all dynamically-added DomainStates.
  //
  // If |sni_enabled| is true, searches the static pins defined for
  // SNI-using hosts as well as the rest of the pins.
  //
  // If |host| matches both an exact entry and is a subdomain of another
  // entry, the exact match determines the return value.
  //
  // Note that this method is not const because it opportunistically removes
  // entries that have expired.
  bool GetStaticDomainState(const std::string& host,
                            bool sni_enabled,
                            DomainState* result);

  // Returns true and updates |*result| iff there is a dynamic DomainState for
  // |host|.
  //
  // |GetDynamicDomainState| is identical to |GetDomainState| except that it
  // searches only the dynamically-added transport security state, ignoring
  // all statically-defined DomainStates.
  //
  // If |host| matches both an exact entry and is a subdomain of another
  // entry, the exact match determines the return value.
  //
  // Note that this method is not const because it opportunistically removes
  // entries that have expired.
  bool GetDynamicDomainState(const std::string& host, DomainState* result);

  // The set of hosts that have enabled TransportSecurity.
  DomainStateMap enabled_hosts_;

  Delegate* delegate_;

  DISALLOW_COPY_AND_ASSIGN(TransportSecurityState);
};

}  // namespace net

#endif  // NET_HTTP_TRANSPORT_SECURITY_STATE_H_
