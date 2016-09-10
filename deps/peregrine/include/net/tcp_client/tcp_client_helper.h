#pragma once

#include "net/base/net_export.h"

#include <stddef.h>

namespace net {

enum ResCode {
  Res_OK = 0,
  Res_TooBigToSend,       // 目前发送缓冲区满，无法发送，要等待OnSend事件
  Res_InitFail,           // 初始化错误，一般很少出现
  Res_InvalidSock,        // socket对象不能使用，可能是网络已经出错了
  Res_ConnectTimeout,     // 连接超时
  Res_ProxyUnknown,       // 不认识的代理服务器类型,
  Res_ProxyAuthFail,      // 代理服务器认证出错,
  Res_RecvTimeout,        // 接收数据超时
  Res_RecvDataError,      // 接收数据出现错误
  Res_InCompatiProto,     // 协议不兼容 
  Res_LocalError,         // 本地比较罕见的错误,
  Res_NetworkError,       // 其它错误,要根据详细错误码判定
  Res_ConnectionClose,    // 连接被关闭

  Res_ErrNum              // 错误总数 
};

class INetConnectCallback {
public:
  virtual void OnConnect(ResCode rescode) = 0;
  virtual void OnRecv(const char* pData, size_t size) = 0;
  virtual void OnError(int rescode) = 0;
};

class INetConnect {
public:
  virtual ResCode Connect(const char* host, int port) = 0;

  //发送数据
  virtual ResCode Send(const char* pBuf, size_t size) = 0;

  //关闭连接
  virtual void Close() = 0;

  //是否已经连接上
  virtual bool IsConnected() = 0;
};

NET_EXPORT bool CreateNetConnect(INetConnect** ppConn, INetConnectCallback* pConnCallback);
NET_EXPORT void DestroyNetConnect(INetConnect* pConn);

}// namespace net 

