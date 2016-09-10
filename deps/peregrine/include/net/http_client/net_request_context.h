#ifndef NET_HTTP_CLIENT_REQUEST_CONTEXT_H_
#define NET_HTTP_CLIENT_REQUEST_CONTEXT_H_
#include "base/memory/singleton.h"
#include "base/threading/thread.h"
#include "net/http/http_server_properties_impl.h"
#include "http_transaction_adapter.h"
#include <map>
#include "net/base/auth.h"
#include "base/synchronization/lock.h"
#include "net/proxy/proxy_server.h"


namespace base {
  class SequencedWorkerPool;
}

namespace net {

class HttpTransactionFactory;
class HostResolver;
class CertVerifier;
class TransportSecurityState;
class ProxyService;
class SSLConfigService;
class HttpAuthHandlerFactory;
class HttpServerPropertiesImpl;
class HttpNetworkSession;
class ProxyConfig;
class CookieStore;

class NET_EXPORT NetRequestContext {
 public:
  // Retrieves the Singleton.
  static NetRequestContext* GetInstance();
  
  NetRequestContext();

  ~NetRequestContext();

  base::subtle::Atomic32 GetRequestSequence();

  base::MessageLoop* GetIoMessageLoop();

  void ShutdownMessageLoop();

  scoped_refptr<base::SequencedWorkerPool> GetWorkPool();

  scoped_refptr<HttpTransactionAdapter> CreateHttpTransactionAdapter(
    scoped_refptr<HttpTransactionAdapter::Delegate> delegate,
    base::subtle::Atomic32 id);

  int DeleteHttpTransactionAdapter(base::subtle::Atomic32 id);

  bool IsExistsHttpTransactionAdapter(base::subtle::Atomic32 id);

  void SetProxy(const HTTP_CLIENT_RPOXY_TYPE type,
    const std::wstring& host,
    const std::wstring& port,
    const std::wstring& domain,
    const std::wstring& user,
    const std::wstring& password);

  AuthCredentials GetAuthCredentials();

  HttpTransactionFactory* GetTransactionFactory();
  ProxyServer GetProxyServer();

  CookieStore* cookie_store() const;

  void AsynGetCookie(std::string const& url, std::string const& key, 
    base::Callback<void(std::string)> callback);

 public:
  static uint64 flags_;

 private:
  void SetFromIEConfig(ProxyConfig& config, std::string & strProxy);
  void OnCookiesLoaded(std::string const& key, 
    base::Callback<void(std::string)> callback,
    const std::string& cookie_line);

 private:
  base::Thread net_io_thread_;
  scoped_ptr<net::HostResolver> host_resolver_;
  scoped_ptr<net::CertVerifier> cert_verifier_;
  scoped_ptr<net::TransportSecurityState> transport_security_state_;
  scoped_ptr<net::ProxyService> proxy_service_;
  scoped_refptr<net::SSLConfigService> ssl_config_service_;
  scoped_ptr<net::HttpAuthHandlerFactory> http_auth_handler_factory_;
  scoped_ptr<net::HttpServerPropertiesImpl> http_server_properties_;
  scoped_ptr<net::HttpTransactionFactory> factory_;
  scoped_refptr<net::HttpNetworkSession> network_session_;
  volatile base::subtle::Atomic32 http_request_sequence_;
  scoped_refptr<base::SequencedWorkerPool> worker_pool_;
  std::map<base::subtle::Atomic32, scoped_refptr<HttpTransactionAdapter>> http_transactions_;
  AuthCredentials auth_credentials_;
  scoped_refptr<CookieStore> cookie_store_;  
  ProxyServer proxy_server_;
};
}

#endif // NET_HTTP_CLIENT_REQUEST_CONTEXT_H_
