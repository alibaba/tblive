#pragma once

#include "tcp_client_helper.h"

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "net/socket/client_socket_handle.h"
#include "net/base/host_port_pair.h"
#include "net/proxy/proxy_info.h"
#include "net/base/net_log.h"
#include "net/http/http_network_session.h"
#include "net/base/completion_callback.h"
#include "net/base/io_buffer.h"

namespace net {

// tcp客户端请求的封装
class TcpClientRequestDelegate : public INetConnect, public base::RefCountedThreadSafe<TcpClientRequestDelegate> {
public:
  TcpClientRequestDelegate(INetConnectCallback * net_connect_callback);

protected:
  virtual ~TcpClientRequestDelegate();
  friend class base::RefCountedThreadSafe<TcpClientRequestDelegate>;
  
public:
  // INetConnect
  virtual ResCode Connect(const char* host, int port) OVERRIDE;
  virtual ResCode Send(const char* pBuf, size_t size) OVERRIDE;
  virtual void Close() OVERRIDE;
  virtual bool IsConnected() OVERRIDE;

private:
  void DoRead();
  void OnReadIOComplete(int result);

  void DoSend(std::string data);
  int WriteBuffer(scoped_refptr<net::DrainableIOBuffer> data);
  void DidWrite(scoped_refptr<net::DrainableIOBuffer> data, int result);

  void InitInternalSocket(std::string host, int port);
  void OnSocketInited(int result);
  void OnConnected(int result);

  void OnError();
  void OnClose();

private:
  static const int READ_BUFFER_SIZE = 2048;

  INetConnectCallback * net_connect_callback_;

  bool connect_inited_;
  scoped_ptr<ClientSocketHandle> connection_;
  HostPortPair dest_host_port_pair_;
  HttpNetworkSession * network_session_;
  ProxyInfo proxy_info_;
  BoundNetLog bound_net_log_;
  
  // variables below are used in a single IO thread

  scoped_refptr<IOBuffer> read_buffer_;

  // cache data to buffer if during write
  bool during_writing_;
  std::string cached_write_buf_;
  scoped_refptr<StringIOBuffer> write_io_buffer_;

  base::WeakPtrFactory<TcpClientRequestDelegate> weak_ptr_factory_;
};

}// namespace net
