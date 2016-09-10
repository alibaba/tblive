#ifndef NET_HTTP_FETCH_URL_H_
#define NET_HTTP_FETCH_URL_H_
#include "http_client_request_delegate.h"
#include "base/threading/sequenced_worker_pool.h"
namespace net {
class HttpClientFetchUrlDelegate
  : public HttpClientRequestDelegate
{
 public:
  static void StartHttpTransction(
    const base::subtle::Atomic32 id,
    const std::wstring& url,
                                  const base::Callback<void(std::string, int)> callback );
	
  static void StartHttpTransctionWithTimeout(const base::subtle::Atomic32 id,
								  const std::wstring& url,
								  int timeout,
								  const base::Callback<void(std::string, int)> callback );
	
  HttpClientFetchUrlDelegate(const base::Callback<void(std::string, int)>& callback,
                             const base::subtle::Atomic32 id);
  void OnTimeOut();
	
protected:
  virtual ~HttpClientFetchUrlDelegate();
  friend class HttpClientRequestDelegate;

 protected:
  virtual void DoCallback() OVERRIDE ;
  
 private:
  base::Callback<void(std::string, int)> callback_;
};
}
#endif  // NET_HTTP_FETCH_URL_H_
