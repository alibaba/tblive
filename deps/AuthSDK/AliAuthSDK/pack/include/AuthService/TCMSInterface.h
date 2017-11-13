
#pragma once

#include <set>
#include <string>
#include <base/bind.h>
#include <base/memory/ref_counted.h>
#include <base/memory/iref_counted.h>
#include <net/http_client/http_client_helper.h>
#include <core/PrgString.h>


#ifdef	__TCMSLIB_EXPORT__
#define TCMSLIB	__declspec(dllexport)
#else	
#define TCMSLIB
#endif	

#ifdef	__cplusplus

#define TCMS_EXTERN	extern TCMSLIB

#else	// #ifdef __cplusplus

// 想在C里面用用这些函数？想的美
#pragma message( "Please use this file in C++") 
#pragma error

#endif	// #ifdef __cplusplus


namespace tcmslib
{
    
enum CONNECT_STATE
{
    CONNECT_STATE_DISCONNECT = 0,
    CONNECT_STATE_CONNECT,
    CONNECT_STATE_STOP,
    CONNECT_STATE_KICKED,
    CONNECT_STATE_TIMEOUT,
    CONNECT_STATE_ERROR,
};

enum SEND_STATE
{
    SEND_STATE_OK = 0,
    SEND_STATE_TIMEOUT,
    SEND_STATE_ERROR,
};

enum REAUTH_STATE
{
    REAUTH_STATE_SUCCESS = 0,
    REAUTH_STATE_FAIL,
    REAUTH_STATE_TIMEOUT
};

struct NetProxy
{
    NetProxy()
        : type(net::HTTP_CLIENT_RPOXY_NONE)
    {
    };

    net::HTTP_CLIENT_RPOXY_TYPE type;
    PrgString host;
    PrgString port;
    PrgString domain;
    PrgString user;
    PrgString password;
};

struct ReqInfo
{
    int nsession;
    std::string strInterface;
    std::string strMethod;
    std::string strData;
};

// 用于发送异步请求后，回调发送结果  
typedef base::Callback<void(SEND_STATE, const std::string&, const std::string&, const std::string&, const ReqInfo&)> TCMSSendCallback;

// 用于监听TCM推送数据及连接状态，对于Req请求的Rsp不会收到OnNotify调用，需要使用ITCMSSendCallback  
class ITCMSListener
{
public:
    //************************************************************************
    // Description:TCM物理连接状态  
    // Returns:    void
    // Parameter:  CONNECT_STATE state
    // Parameter:  const std::string & data
    //************************************************************************
    virtual void OnConnectStatus(CONNECT_STATE state, const std::string& data){};
    //************************************************************************
    // Description:业务虚链接状态  
    // Returns:    void
    // Parameter:  CONNECT_STATE state
    // Parameter:  const std::string & data
    //************************************************************************
    virtual void OnVConnectStatus(CONNECT_STATE state, const std::string& data){};
//     //************************************************************************
//     // Description:断线重连，重新验证状态  
//     // Returns:    void
//     // Parameter:  REAUTH_STATE state
//     // Parameter:  int64 time
//     //************************************************************************
//     virtual void OnReAuthStatus(REAUTH_STATE state, int64 time){};
    //************************************************************************
    // Description:服务器推送消息  
    // Returns:    void
    // Parameter:  const std::string & _interface
    // Parameter:  const std::string & _method
    // Parameter:  const std::string & data
    //************************************************************************
    virtual void OnNotify(const std::string& _interface, const std::string& _method, const std::string& data){};
};

typedef ITCMSListener* TCMSListenerPtr;

class ITCMSChannel
{
public:
	//************************************************************************
	// Description:异步调用，发起物理连接  
	// Returns:    void
	//************************************************************************
	virtual void Start(const NetProxy &proxy, const PrgString& strDns, const PrgString& strDataPath, const PrgString& strServerAddr) = 0;
	//************************************************************************
	// Description:异步调用，关闭物理连接  
	// Returns:    void
	//************************************************************************
	virtual void Stop() = 0;

    //************************************************************************
    // Description:同步调用，通过服务名来注册vconnect id  
    // Returns:    void
    // Parameter:  std::string name
    // Parameter:  int & nSessionId
    virtual void RegisterVConn(std::string name, int &nSessionId) = 0;

    //************************************************************************
    // Description:异步调用，添加Listener  
    // Returns:    void
    // Parameter:  TCMSListenerPtr listener
    // Parameter:  int nSessionId
    //************************************************************************
    virtual void AddListener(TCMSListenerPtr listener, int nSessionId) = 0;
    //************************************************************************
    // Description:异步调用，移除Listener  
    // Returns:    void
    // Parameter:  int nSessionId
    //************************************************************************
    virtual void RemoveListener(int nSessionId) = 0;
    //************************************************************************
    // Description:异步调用，创建虚连接  
    // Returns:    void
    // Parameter:  int nSessionId，AddListener返回值  
    // Parameter:  const std::string & clusterName，目前不用，暂时为""  
    // Parameter:  const std::string & routeName，值可以为upush，xpush，ww，wx,标注的是项目的名字，  
    // 其中upush就是u项目的含义，服务器根据这个值连接项目对应的业务服务器  
    // Parameter:  const std::string & alllotKey，用于TCM找到上次连接的服务器，基于uid的为用户名，deviceid的为设备唯一标示  
    //************************************************************************
    virtual void CreateVConn(int nSessionId, const std::string& clusterName, const std::string& routeName, const std::string& alllotKey) = 0;
    //************************************************************************
    // Description:异步调用，销毁虚连接  
    // Returns:    void
    // Parameter:  int nSessionId
    //************************************************************************
    virtual void DestroyVConn(int nSessionId) = 0;
// 	//************************************************************************
// 	// Description: Not implement
// 	// Returns:    void
// 	// Parameter:  int nSessionId
// 	// Parameter:  const std::string & _interface
// 	// Parameter:  const std::string & _method
// 	// Parameter:  const std::string & req
// 	// Parameter:  const std::string & rsp
// 	// Parameter:  int timeout
// 	//************************************************************************
// 	virtual void SyncSend(int nSessionId, const std::string& _interface, const std::string& _method, const std::string& req, const std::string& rsp, int timeout = 10) = 0;
	//************************************************************************
	// Description:异步调用，发送请求  
	// Returns:    void
	// Parameter:  int nSessionId
	// Parameter:  const std::string & _interface
	// Parameter:  const std::string & _method
	// Parameter:  const std::string & req
	// Parameter:  TCMSSendCallback cb
	// Parameter:  int timeout
	//************************************************************************
	virtual void AsynSend(int nSessionId, const std::string& _interface, const std::string& _method, const std::string& req, TCMSSendCallback cb, int timeout = 10) = 0;
	//************************************************************************
	// Description:异步调用，发送Notify请求，即无响应请求  
	// Returns:    void
	// Parameter:  int nSessionId
	// Parameter:  const std::string & _interface
	// Parameter:  const std::string & _method
	// Parameter:  const std::string & req
	//************************************************************************
	virtual void NotifySend(int nSessionId, const std::string& _interface, const std::string& _method, const std::string& req) = 0;
//     //************************************************************************
//     // Description:异步调用，验证成功后掉线，重新发送验证请求  
//     // 服务器考虑到统一处理验证后的断线重连及性能，所以把reAuth的机制加在TCM上，所以客户端也存在相应的逻辑  
//     // 服务端使用clusterName+uniqId找到之前的登录信息  
//     // Returns:    void
//     // Parameter:  int nSessionId
//     // Parameter:  const std::string & clusterName,值为uid  
//     // Parameter:  const std::string & uniqId，值为reAuth的用户Id  
//     //************************************************************************
//     virtual void ReAuth(int nSessionId, const std::string& clusterName, const std::string& uniqId, const std::string& version) = 0;
};

 TCMS_EXTERN ITCMSChannel* GetTCMSChannel();

}
