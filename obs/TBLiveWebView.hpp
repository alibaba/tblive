#pragma once

#include <QWidget>
#include <qwebengineview.h>

#include "tblive_sdk/SDKTblive.h"


class TBLiveWebView : public QWebEngineView
{
  Q_OBJECT
private:

public:
  TBLiveWebView(QWidget *parent = 0);
  virtual ~TBLiveWebView();

  void Init();
  void SetStreamingStatus(TBLiveStreamStatus status);

private slots:
  void OnLoadFinish(bool finish);

protected:
  void contextMenuEvent(QContextMenuEvent* event) override;



private:
  SDKTBLive* root_;

};