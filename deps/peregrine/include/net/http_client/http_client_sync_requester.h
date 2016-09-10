#ifndef NET_HTTP_CLIENT_SYNC_REQUESTER_H_
#define NET_HTTP_CLIENT_SYNC_REQUESTER_H_
#include "base/synchronization/waitable_event.h"
#include "base/memory/ref_counted.h"

namespace net {
template<typename R>
class HttpClientSyncRequester
  : public base::RefCountedThreadSafe<HttpClientSyncRequester<R>>{
public:
  HttpClientSyncRequester() 
    : event_(false, false){}
  ~HttpClientSyncRequester(){}
  bool Wait(int timeout) {
    return event_.TimedWait(base::TimeDelta::FromMilliseconds(timeout));
  }
  void OnComplete(const R response) {
    response_ = response;
    event_.Signal();
  }
  R response_;
  base::WaitableEvent event_;
};
}
#endif//NET_HTTP_CLIENT_SYNC_REQUESTER_H_
