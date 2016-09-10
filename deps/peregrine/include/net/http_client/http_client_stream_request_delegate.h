#ifndef NET_HTTP_CLIENT_STREAM_REQUEST_DELEGATE_H_
#define NET_HTTP_CLIENT_STREAM_REQUEST_DELEGATE_H_
#include "http_client_helper.h"
#include "net_request_context.h"
#include "http_transaction_adapter.h"
#include "net/base/net_errors.h"
#include "base/threading/sequenced_worker_pool.h"

namespace net {

class HttpClientStreamRequestDelegate
  : public HttpTransactionAdapter::Delegate
{
 public:
  static void StartHttpTransction(
    const base::subtle::Atomic32 id,
    const HttpRequest& req,
    const scoped_refptr<StreamRequestDelegate>& delegate ) {
      scoped_refptr<HttpTransactionAdapter> adapter= 
        NetRequestContext::GetInstance()->CreateHttpTransactionAdapter(
        make_scoped_refptr(new HttpClientStreamRequestDelegate(delegate, id)),
        id);
      adapter->Start(req);
  }

  HttpClientStreamRequestDelegate(const scoped_refptr<StreamRequestDelegate>& delegate,
                                  const base::subtle::Atomic32 id);
protected:
  virtual ~HttpClientStreamRequestDelegate();
  friend class HttpTransactionAdapter::Delegate;
  
public:

  virtual void OnReceivedHeaders(const scoped_refptr<HttpResponseHeaders>& headers) OVERRIDE;
  
  virtual void OnReceivedData(const std::string& data) OVERRIDE;
  
  virtual void OnError(int result) OVERRIDE;
  
  virtual void OnRequestComplete() OVERRIDE;
  
protected:
  base::subtle::Atomic32 id_;
  base::SequencedWorkerPool::SequenceToken seq_token_;

 private:
  scoped_refptr<StreamRequestDelegate> delegate_;
};
}
#endif  // NET_HTTP_CLIENT_STREAM_REQUEST_DELEGATE_H_
