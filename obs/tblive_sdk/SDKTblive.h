#pragma once

#include <QObject>
#include <QVariant>


enum TBLiveStreamStatus {
	TSS_START,
	TSS_PAUSE,
	TSS_STOP,
};


class SDKTBLive : public QObject
{
	Q_OBJECT

public:
	SDKTBLive(QObject* parent);

	void SetStreamingStatus(TBLiveStreamStatus status);

public slots:
	// request
	void reqSetRtmpUrl(QString reqid, QString url);
	void reqGetCurrentStatus(QString reqid);
	void reqSetSourceConfig(QString reqid, QString sourceConfigJson);

signals:
	// response
	void onRspSetRtmpUrl(QString reqid, QString url);
	void onRspGetCurrentStatus(QString reqid, int status);
	void onRspSetSourceConfig(QString reqid);

signals:
	// notify
	void onNotifyStartStreaming();
	void onNotifyPauseStreaming();
	void onNotifyStopStreaming();
};
