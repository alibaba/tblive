#ifndef NET_HTTP_CLIENT_REQUEST_DELEGATE_H_
#define NET_HTTP_CLIENT_REQUEST_DELEGATE_H_

#include "http_client_helper.h"
#include "net_request_context.h"
#include "http_transaction_adapter.h"
#include "net/base/net_errors.h"
#include "base/threading/sequenced_worker_pool.h"
#include "base/time/time.h"

namespace net {

class HttpClientRequestDelegate
  : public HttpTransactionAdapter::Delegate
{
 public:
  static void StartHttpTransction(
    const base::subtle::Atomic32 id,
    const HttpRequest& req,
                                  const base::Callback<void(HttpResponse)> callback );
  static void StartHttpTransctionWithTimeout(
									const base::subtle::Atomic32 id,
									const HttpRequest& req,
								    int timeout,
									const base::Callback<void(HttpResponse)> callback );
	
  HttpClientRequestDelegate(const base::Callback<void(HttpResponse)>& callback,
                            const base::subtle::Atomic32 id);

  void OnTimeOut();
	
  virtual void OnReceivedHeaders(const scoped_refptr<HttpResponseHeaders>& headers) OVERRIDE;
  
  virtual void OnReceivedData(const std::string& data) OVERRIDE ;
  
  virtual void OnError(int result) OVERRIDE ;
  
  virtual void OnRequestComplete() OVERRIDE ;
  
protected:
  virtual void DoCallback();
  
protected:
  virtual ~HttpClientRequestDelegate();
  friend class HttpTransactionAdapter::Delegate;
  
 protected:
  HttpResponse response_;
  const base::subtle::Atomic32 id_;
  base::TimeTicks begin_;
  
  base::Callback<void(HttpResponse)> callback_;
};
}
#endif  // NET_HTTP_CLIENT_REQUEST_DELEGATE_H_
