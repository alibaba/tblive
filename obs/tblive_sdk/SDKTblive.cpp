
#include "SDKTblive.h"

#include "tblive_sdk/biz.h"
#include "core/PrgString.h"
#include "base/strings/utf_string_conversions.h"

SDKTBLive::SDKTBLive(QObject* parent)
  : QObject(parent)
{
}

void SDKTBLive::reqSetRtmpUrl(QString reqid, QString url)
{
	PrgString wstr = url.toStdWString();
	biz::setRtmpUrl(base::WideToUTF8(wstr.GetString()));

	emit onRspSetRtmpUrl(reqid, url);
}

void SDKTBLive::reqGetCurrentStatus(QString reqid)
{
	int status = biz::getStreamimgStatus();

	emit onRspGetCurrentStatus(reqid, status);
}

void SDKTBLive::reqSetSourceConfig(QString reqid, QString sourceConfigJson)
{
	biz::setSourceConfig(sourceConfigJson.toStdWString());
	emit onRspSetSourceConfig(reqid);
}

void SDKTBLive::SetStreamingStatus(TBLiveStreamStatus status)
{
	switch (status)
	{
	case TSS_START:
		emit onNotifyStartStreaming();
		break;

	case TSS_PAUSE:
		emit onNotifyPauseStreaming();
		break;

	case TSS_STOP:
		emit onNotifyStopStreaming();
		break;

	default:
		break;
	}
}