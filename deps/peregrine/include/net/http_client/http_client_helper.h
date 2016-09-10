#ifndef NET_HTTP_CLIENT_HELPER_H_
#define NET_HTTP_CLIENT_HELPER_H_

#include "net/http/http_request_headers.h"
#include "net/http/http_response_headers.h"
#include "base/callback.h"

#include <set>
#include <string>
#include <map>

namespace net {
  
typedef int32 HTTP_CLIENT_REQUEST_HANDLE;
//flags
enum HTTP_CLIENT_FLAG {
  //ignore all certificate' errors in https
  HTTP_CLIENT_FLAG_IGNORE_ALL_CERT_ERRORS= 1<<0,
  //force using spdy
  HTTP_CLIENT_FLAG_FORCE_USING_SPDY= 1<<1,
};

//proxy type
enum HTTP_CLIENT_RPOXY_TYPE {
  HTTP_CLIENT_RPOXY_NONE = 0,
  HTTP_CLIENT_RPOXY_SOCKS5,
  HTTP_CLIENT_RPOXY_HTTP,
  HTTP_CLIENT_RPOXY_IE,
};

struct NET_EXPORT HttpPart {
  std::wstring name;
  std::wstring value;
  std::wstring content_type;
};

struct NET_EXPORT HttpFilePart {
  HttpFilePart();
  ~HttpFilePart();
  
  std::wstring file_name;
  std::wstring name;
  std::string value;
  std::wstring content_type;
};

struct NET_EXPORT HttpRequest {
  HttpRequest();
  ~HttpRequest();
  
  // The requested method.
  std::wstring method;

  // The requested URL.
  std::wstring url;

  //The requested headers.
  std::set<std::pair<std::string, std::string>> headers;

  //The requested body data
  std::string data;

  bool disable_redirects;
};

class NET_EXPORT HttpResponse {
public:
  HttpResponse();
  ~HttpResponse();
  
  //The return code
  int result;

  // The parsed response headers and status line.
  scoped_refptr<HttpResponseHeaders> headers;

  //The response body data
  std::string data;

  //time consuming with Milliseconds
  int time;

  std::wstring GetDataInUnicode();

  std::string GetData();

  std::string GetRawData();
};

class NET_EXPORT StreamRequestDelegate
   : public base::RefCountedThreadSafe<StreamRequestDelegate>{
public:
  StreamRequestDelegate(){}
     
  //响应http头，头部内容存在headers中
  virtual void OnReceivedHeaders(const scoped_refptr<HttpResponseHeaders>& headers) = 0;

  //接收http的body数据，data存储返回的数据
  virtual void OnReceivedData(const std::string& data) = 0;

  //http连接出错，result为错误码
  virtual void OnError(int result) = 0;

  //完成http请求
  virtual void OnRequestComplete() = 0;
     
protected:
  virtual ~StreamRequestDelegate() {}
  friend class base::RefCountedThreadSafe<StreamRequestDelegate>;
};

//这是个全局函数可以动态的修改http请求的属性，例如忽略https的证书错误，强制启用spdy等,建议先调用
//nFlags 需要修改的标记
NET_EXPORT void SetFlags(const uint64& flags);

//根据url同步抓取内容
//url 资源地址
//timeout 超时时间毫秒为单位
//response 服务器响应的内容
//错误码或者http状态码，注意返回200表示请求成功
NET_EXPORT int FetchUrl(const std::wstring& url,
    const int timeout,
    std::string& response);

//异步抓取url的内容
//url 资源地址
//timeout 超时时间毫秒为单位
//callback 回调通知，两个参数，string为响应的值，int类型是错误码或者http状态码
//成功返回请求标识，可以通过方法CloseRequest终止一个请求，失败返回NULL
NET_EXPORT HTTP_CLIENT_REQUEST_HANDLE AsynFetchUrl(const std::wstring& url,
    const base::Callback<void(std::string, int)>& callback);

// 支持超时的AsynFetchUrl
NET_EXPORT HTTP_CLIENT_REQUEST_HANDLE AsynFetchUrl(const std::wstring& url,
	const int timeout,
	const base::Callback<void(std::string, int)>& callback );
	
//同步发送一个http请求
//req 请求的内容包括url，method，http头部等等
//timeout 超时时间毫秒为单位
//rsp 服务器响应的内容，包括错误码, 响应头，响应body等等
NET_EXPORT void SendRequest(const HttpRequest& req,
    const int timeout,
    HttpResponse& rsp);

//异步发送一个http请求
//req 请求的内容包括url，method，http头部等等
//timeout 超时时间毫秒为单位
//callback 返回响应的内容
//成功返回请求标识，可以通过方法CloseRequest终止一个请求，失败返回NULL
NET_EXPORT HTTP_CLIENT_REQUEST_HANDLE AsynSendRequest(const HttpRequest& req,
	const base::Callback<void(HttpResponse)>& callback);

// 支持超时的AsynSendRequest
NET_EXPORT HTTP_CLIENT_REQUEST_HANDLE AsynSendRequest(const HttpRequest& req,
	const int timeout,
	const base::Callback<void(HttpResponse)>& callback);
	
//建立一个http的长连接
//req 请求的内容包括url，method，http头部等等
//delegate 接收各种事件的通知
//成功返回请求标识，可以通过方法CloseRequest终止一个请求，失败返回NULL
NET_EXPORT HTTP_CLIENT_REQUEST_HANDLE StreamForRequest(const HttpRequest& req,
  const scoped_refptr<StreamRequestDelegate>& delegate);

//下载文件
//url 文件地址
//dst_path 文件的保存路径
//delegate 接收各种事件的通知
//成功返回请求标识，可以通过方法CloseRequest终止一个请求，失败返回NULL
NET_EXPORT HTTP_CLIENT_REQUEST_HANDLE DownLoadFileForRequest(const std::wstring& url,
  const std::wstring dst_path,
  const scoped_refptr<StreamRequestDelegate>& delegate);
  
NET_EXPORT void DownLoadFileForRequest(const std::wstring& url,
   const std::wstring dst_path,
   const base::Callback<void(HttpResponse)>& callback );
  

//通过表单提交的方式同步上传文件
//req http请求
//http_parts 表单里面的键值对
//http_file_parts 文件属性以及文件内容
//timeout 超时时间毫秒为单位
//成功返回 OK 失败返回错误码
NET_EXPORT void UploadFileByMultiPart(const HttpRequest& req, 
                    const std::vector<HttpPart>& http_parts,
                    const std::vector<HttpFilePart>& http_file_parts,
                    const int timeout,
                    HttpResponse& rsp);

//通过表单提交的方式异步上传文件
//req http请求
//http_parts 表单里面的键值对
//http_file_parts 文件属性以及文件内容
//callback 回调操作上传结果
//成功返回请求标识，可以通过方法CloseRequest终止一个请求，失败返回NULL
NET_EXPORT HTTP_CLIENT_REQUEST_HANDLE AsynUploadFileByMultiPart(const HttpRequest& req, 
                      const std::vector<HttpPart>& http_parts,
                     const std::vector<HttpFilePart>& http_file_parts,
                     const base::Callback<void(HttpResponse)>& callback);

//异步终止一个http请求
//handle 由上面几个异步请求返回的标识
//callback 回调通知
NET_EXPORT void CloseRequest(const HTTP_CLIENT_REQUEST_HANDLE handle,
                     const base::Callback<void(int)>& callback);

//全局函数，设置代理
//type 代理的类型
//host 服务器地址
//port 服务器端口
//domain 用户域
//user 用户名
//password 密码
NET_EXPORT void SetProxy(const HTTP_CLIENT_RPOXY_TYPE type,
                      const std::wstring& host,
                      const std::wstring& port,
                      const std::wstring& domain,
                      const std::wstring& user,
                      const std::wstring& password);

    
NET_EXPORT void AsynGetCookie(std::string const& url, std::string const& key,
                              base::Callback<void(std::string)> callback);
NET_EXPORT bool GetCookie(std::string const& url, std::string const& key, std::string * val);
    
NET_EXPORT void ShutdownNetIO();

}// namespace net 
#endif//NET_HTTP_CLIENT_HELPER_H_
