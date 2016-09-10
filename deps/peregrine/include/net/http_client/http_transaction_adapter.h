#ifndef NET_HTTP_TRANSACTION_ADAPTER_H_
#define NET_HTTP_TRANSACTION_ADAPTER_H_
#include "base/memory/ref_counted.h"
#include "net/http/http_transaction.h"
#include "base/threading/non_thread_safe.h"
#include "http_client_helper.h"

namespace net {

class IOBuffer;
class HttpResponseHeaders;
class UploadDataStream;

typedef base::RefCountedData<bool> SharedBoolean;

class NET_EXPORT HttpTransactionAdapter
  : public base::RefCounted<HttpTransactionAdapter>,
    NON_EXPORTED_BASE(public base::NonThreadSafe) {
 public:

  class NET_EXPORT_PRIVATE Delegate 
    : public base::RefCounted<Delegate> {
  public:
    Delegate(){}

    virtual void OnReceivedHeaders(const scoped_refptr<HttpResponseHeaders>& headers) = 0;

    virtual void OnReceivedData(const std::string& data) = 0;

    virtual void OnError(int result) = 0;

    virtual void OnRequestComplete() = 0;
      
  protected:
    virtual ~Delegate() {}
    friend class base::RefCounted<Delegate>;
  };

  void Start(const HttpRequest& request);

  void Stop();

  HttpTransactionAdapter(scoped_ptr<HttpTransaction>& transaction,
    scoped_refptr<HttpTransactionAdapter::Delegate>& delegate);

protected:
  virtual ~HttpTransactionAdapter();
  friend class base::RefCounted<HttpTransactionAdapter>;
      
private:
   enum State {
     STATE_READ_HEADERS,
     STATE_READ_HEADERS_AND_SAVE_COOKIE,
     STATE_READ_HEADERS_COMPLETE,
     STATE_READ_HEADERS_WITH_AUTH,
     STATE_READ_HEADERS_WITH_AUTH_COMPLETE,
     STATE_READ_BODY,
     STATE_READ_BODY_COMPLETE,
     STATE_NONE
   };
   // Runs the state transition adapter loop.
  int DoLoop(int result);

  void OnIoComplete(int result);

  int DoReadHeaders();

  int DoReadHeadersAndSaveCookies(int result);

  int DoReadHeadersComplete(int result);

  int DoReadHeadersWithAuth();

  int DoReadHeadersWithAuthComplete(int result);

  int DoReadBody();

  int DoReadBodyComplete(int result);

  void DelayDeleteTransaction();

  void AddCookieHeaderAndStart();
  void DoStartTransaction();
  HttpResponseHeaders* GetResponseHeaders() const;
  void SaveNextCookie();
  void OnCookiesLoaded(const std::string& cookie_line);
  void OnCookieSaved(scoped_refptr<SharedBoolean> save_next_cookie_running, 
    scoped_refptr<SharedBoolean> callback_pending, bool cookie_status);

private:
  scoped_refptr<net::IOBuffer> buffer_;
  scoped_ptr<net::HttpTransaction> transaction_;
  static const int kBufferSize = (16 * 1024);
  scoped_refptr<HttpTransactionAdapter::Delegate> delegate_;
  scoped_ptr<HttpRequestInfo> request_info_;
  scoped_ptr<UploadDataStream> upload_data_stream_;
  State next_state_;
  std::string upload_content_;
  bool disable_redirects_;

  std::vector<std::string> response_cookies_;
  size_t response_cookies_save_index_;
  base::Time response_date_;
};


}

#endif  // NET_HTTP_TRANSACTION_ADAPTER_H_
