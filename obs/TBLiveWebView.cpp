
#include "TBLiveWebView.hpp"

#include <QtWebChannel/QtWebChannel>
#include <QWebEngineProfile>

TBLiveWebView::TBLiveWebView(QWidget *parent)
  : QWebEngineView(parent)
  , root_( new SDKTBLive(this) )
{
  connect(this, SIGNAL(loadFinished(bool)), this, SLOT(OnLoadFinish(bool)));

  QWebChannel *channel = new QWebChannel(this);
  channel->registerObject(QStringLiteral("tblive"), root_);
  page()->setWebChannel(channel);

  // userAgent
  page()->profile()->setHttpUserAgent(page()->profile()->httpUserAgent() + QStringLiteral(" tblive/1.0.0"));
}

TBLiveWebView::~TBLiveWebView()
{}

void TBLiveWebView::Init()
{
}

void TBLiveWebView::SetStreamingStatus(TBLiveStreamStatus status)
{
	root_->SetStreamingStatus(status);
}

void TBLiveWebView::contextMenuEvent(QContextMenuEvent* event)
{
  return QWebEngineView::contextMenuEvent(event);
}

void TBLiveWebView::OnLoadFinish(bool finish)
{
}