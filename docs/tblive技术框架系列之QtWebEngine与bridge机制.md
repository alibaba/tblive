tblive技术框架系列之：QtWebEngine与bridge机制
====

Qt5.6中使用的webview是基于chromium45来实现的，叫做QtWebEngine，之前使用QtWebkit已经被废弃，因此webview的兼容性也非常不错；不过由于引入了多进程架构，导致bridge机制比较复杂，需要引入WebChannel的概念，底层是使用websocket来实现的。


### 使用webview

```c++
QMainWindow * widget = new QMainWindow(this);
QWebEngineView * webview = new QWebEngineView(this);
widget->setCentralWidget(m_hideWebview);
widget->show();

// load url
std::wstring url = L"https://my.taobao.com/";
webview->load(QString::fromStdWString(destUrl.GetString()));

// set userAgent
webview->page()->profile()->setHttpUserAgent(page()->profile()->httpUserAgent() + QStringLiteral(" tblive/1.0.0"));
```

### bridge机制

1、创建sdk对象

```c++
class SDKTBLive : public QObject
{
  Q_OBJECT

public:
  SDKTBLive(QObject* parent);

  void SetStreamingActive( bool active );

public slots:
  // request
  void reqSetRtmpUrl(QString reqid, QString url);
  void reqGetCurrentStatus(QString reqid);

signals:
  // response
  void onRspSetRtmpUrl(QString reqid, QString url);
  void onRspGetCurrentStatus(QString reqid, int status);

signals:
  // notify
  void onNotifyStartStreaming();
  void onNotifyStopStreaming();
};
```

2、设置WebChannel

```c++
QWebChannel *channel = new QWebChannel(this);
channel->registerObject(QStringLiteral("tblive"), new SDKTBLive(this));

webview->page()->setWebChannel(channel);
```

3、前端页面对接

1. 需要引入 qwebchannel.js
2. 需要初始化QWebChannel，来获取注入的对象tblive

```javascript
    document.addEventListener("DOMContentLoaded", function () {
        new QWebChannel(qt.webChannelTransport, function (channel) {
            window.tblive = channel.objects.tblive;
            initTblive();
        });
    });
```

参考：http://doc.qt.io/qt-5/qtwebchannel-javascript.html


### 远程调试

1. 需要以命令行参数启动，如：--remote-debugging-port=9000
2. 然后在浏览器中输入 `http://localhost:9000` 即可调试

注意：QtWebEngine5.6 在[chrome升级到50后无法远程调试，需要降低到49版本](https://bugreports.qt.io/browse/QTBUG-52602)



### 多进程架构

使用了QtWebEngine的app，自带了chrome的多进程架构，webview会跑在render进程，子进程名：QtWebEngineProcess.exe

因此，QtWebEngineProcess.exe文件也需要和应用一起发布
