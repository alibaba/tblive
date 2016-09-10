#include "volume-control-custom.hpp"
#include "qt-wrappers.hpp"
#include "mute-checkbox.hpp"
#include "slider-absoluteset-style.hpp"
#include <util/platform.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QVariant>
#include <QSlider>
#include <QLabel>
#include <QPainter>
#include <QTimer>
#include <string>
#include <math.h>
#include <QEvent>
#include "commonToolFun.h"
#include "obs-app.hpp"

using namespace std;

void VolControlCustom::OBSVolumeChanged(void *data, float db)
{
	Q_UNUSED(db);
	VolControlCustom *volControl = static_cast<VolControlCustom*>(data);

	QMetaObject::invokeMethod(volControl, "VolumeChanged");
}

void VolControlCustom::OBSVolumeLevel(void *data, float level, float mag,
	float peak, float muted)
{
	VolControlCustom *volControl = static_cast<VolControlCustom*>(data);

	QMetaObject::invokeMethod(volControl, "VolumeLevel",
		Q_ARG( double, mag),
		Q_ARG( double, level),
		Q_ARG( double, peak),
		Q_ARG(bool,  muted));
}

void VolControlCustom::OBSVolumeMuted(void *data, calldata_t *calldata)
{
	VolControlCustom *volControl = static_cast<VolControlCustom*>(data);
	bool muted = calldata_bool(calldata, "muted");

	QMetaObject::invokeMethod(volControl, "VolumeMuted",
			Q_ARG(bool, muted));
}

void VolControlCustom::VolumeChanged()
{
	slider->blockSignals(true);
	slider->setValue((int) (obs_fader_get_deflection(obs_fader) * 100.0f));
	slider->blockSignals(false);
	
	updateText();
}

void VolControlCustom::VolumeLevel( double mag,  double peak,  double peakHold, bool muted)
{
	if (muted) {
		mag = 0.0f;
		peak = 0.0f;
		peakHold = 0.0f;
	}

	slider->setLevels(mag, peak, peakHold);
}

void VolControlCustom::VolumeMuted(bool muted)
{
	if (mute->isChecked() != muted)
		SetMuteCtrlChecked(muted);
}

void VolControlCustom::SetMuted(bool checked)
{
	obs_source_set_muted(source, checked);
	ChangeMuteCtrlToolTip(checked);
}

void VolControlCustom::SliderChanged(int vol)
{
	obs_fader_set_deflection(obs_fader,  double(vol) * 0.01f);
	updateText();

}

void VolControlCustom::updateText()
{
	volLabel->setText(QString::number(obs_fader_get_db(obs_fader), 'f', 1)
			.append(" dB"));
}

QString VolControlCustom::GetName() const
{
	return nameLabel->text();
}

void VolControlCustom::SetName(const QString &newName)
{
	//VolControlCustom的父宽度为240所以这里maxwidth也取240
	QString streElideName = GetElidedText(nameLabel->font(), newName, 240);
	nameLabel->setText(streElideName);
	if (streElideName != newName)
	{
		nameLabel->setToolTip(newName);
	}
}

VolControlCustom::VolControlCustom(OBSSource source_)
	: source        (source_),
	  levelTotal    (0.0f),
	  levelCount    (0.0f),
	  obs_fader     (obs_fader_create(OBS_FADER_CUBIC)),
	  obs_volmeter  (obs_volmeter_create(OBS_FADER_LOG))
{
	QHBoxLayout *volLayout  = new QHBoxLayout();
	QVBoxLayout *mainLayout = new QVBoxLayout();
	QHBoxLayout *textLayout = new QHBoxLayout();
	QHBoxLayout *botLayout  = new QHBoxLayout();

	nameLabel = new QLabel();
	volLabel  = new QLabel();

	//weiqi.mwq:这里把滑动条的值隐藏，通过滑动条的位置就可以看出效果，个人感觉无需显示
	volLabel->hide();

	mute      = new MuteCheckBox();
	slider = new SliderCustom(Qt::Horizontal);

	QFont font = nameLabel->font();
	font.setPointSize(font.pointSize()-1);
	nameLabel->setFont(font);
	volLabel->setFont(font);
	SetName(obs_source_get_name(source));

	slider->setMinimum(0);
	slider->setMaximum(100);

//	slider->setMaximumHeight(13);

	textLayout->setContentsMargins(0, 0, 0, 0);
	textLayout->addWidget(nameLabel);
	textLayout->addWidget(volLabel);
	textLayout->setAlignment(nameLabel, Qt::AlignLeft);
	textLayout->setAlignment(volLabel,  Qt::AlignRight);

	SetMuteCtrlChecked(obs_source_muted(source));

	volLayout->addWidget(mute);
	volLayout->addWidget(slider);
	volLayout->setSpacing(5);

	botLayout->setContentsMargins(0, 0, 0, 0);
	botLayout->setSpacing(0);
	botLayout->addLayout(volLayout);

	//线条分割线设置
	lineLabel = new QLabel();
	lineLabel->setFixedHeight(1);
	lineLabel->setStyleSheet("background-color:#666666;");

	mainLayout->setContentsMargins(4, 4, 4, 4);
	mainLayout->setSpacing(2);
	mainLayout->addItem(textLayout);
	mainLayout->addItem(botLayout);
	mainLayout->addWidget(lineLabel);
	setLayout(mainLayout);

	obs_fader_add_callback(obs_fader, OBSVolumeChanged, this);
	obs_volmeter_add_callback(obs_volmeter, OBSVolumeLevel, this);

	signal_handler_connect(obs_source_get_signal_handler(source),
			"mute", OBSVolumeMuted, this);

	QWidget::connect(slider, SIGNAL(valueChanged(int)),
			this, SLOT(SliderChanged(int)));
	QWidget::connect(mute, SIGNAL(clicked(bool)),
			this, SLOT(SetMuted(bool)));

	obs_fader_attach_source(obs_fader, source);
	obs_volmeter_attach_source(obs_volmeter, source);

	slider->setStyle(new SliderAbsoluteSetStyle(slider->style()));
	/* Call volume changed once to init the slider position and label */
	VolumeChanged();
}

VolControlCustom::~VolControlCustom()
{
	obs_fader_remove_callback(obs_fader, OBSVolumeChanged, this);
	obs_volmeter_remove_callback(obs_volmeter, OBSVolumeLevel, this);

	signal_handler_disconnect(obs_source_get_signal_handler(source),
			"mute", OBSVolumeMuted, this);

	obs_fader_destroy(obs_fader);
	obs_volmeter_destroy(obs_volmeter);
}
void VolControlCustom::SetMuteCtrlChecked(bool bMuted)
{
	mute->setChecked(bMuted);
	ChangeMuteCtrlToolTip(bMuted);
}

void VolControlCustom::ChangeMuteCtrlToolTip(bool bMuted)
{
	if (bMuted)
	{
		mute->setToolTip(QTStr("TBLive.StopMuteTip"));
	}
	else
	{
		mute->setToolTip(QTStr("TBLive.SetMuteTip"));
	}
}

QColor SliderCustom::getBkColor() const
{
	return bkColor;
}

void SliderCustom::setBkColor(QColor c)
{
	bkColor = c;
}

QColor SliderCustom::getMagColor() const
{
	return magColor;
}

void SliderCustom::setMagColor(QColor c)
{
	magColor = c;
}

QColor SliderCustom::getPeakColor() const
{
	return peakColor;
}

void SliderCustom::setPeakColor(QColor c)
{
	peakColor = c;
}

QColor SliderCustom::getPeakHoldColor() const
{
	return peakHoldColor;
}

void SliderCustom::setPeakHoldColor(QColor c)
{
	peakHoldColor = c;
}



void SliderCustom::resetState(void)
{
	setLevels(0.0f, 0.0f, 0.0f);
	if (resetTimer->isActive())
		resetTimer->stop();
}

SliderCustom::SliderCustom(QWidget *parent/* = Q_NULLPTR*/)
	:QSlider(parent)
{
	setMinimumSize(1, 3);

	//Default meter color settings, they only show if there is no stylesheet, do not remove.
	bkColor.setRgb(0xDD, 0xDD, 0xDD);
	magColor.setRgb(0x20, 0x7D, 0x17);
	peakColor.setRgb(0x3E, 0xF1, 0x2B);
	peakHoldColor.setRgb(0x00, 0x00, 0x00);

	resetTimer = new QTimer(this);
	connect(resetTimer, SIGNAL(timeout()), this, SLOT(resetState()));

	resetState();
}

SliderCustom::SliderCustom(Qt::Orientation orientation, QWidget *parent/* = Q_NULLPTR*/)
	: QSlider(orientation,parent)
{
	setMinimumSize(1, 3);

	//Default meter color settings, they only show if there is no stylesheet, do not remove.
	bkColor.setRgb(0xDD, 0xDD, 0xDD);
	magColor.setRgb(0x20, 0x7D, 0x17);
	peakColor.setRgb(0x3E, 0xF1, 0x2B);
	peakHoldColor.setRgb(0x00, 0x00, 0x00);

	resetTimer = new QTimer(this);
	connect(resetTimer, SIGNAL(timeout()), this, SLOT(resetState()));

	resetState();
}

void SliderCustom::setLevels( double nmag,  double npeak,  double npeakHold)
{
	mag = nmag;
	peak = npeak;
	peakHold = npeakHold;

	update();

	if (resetTimer->isActive())
		resetTimer->stop();
	resetTimer->start(1000);
}

void SliderCustom::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);

	//滑动块半径
	const int ellipseR = 5;		

	//绘制底色框
	const int offsetY = size().height() / 2 - 2;
	QRect rect = this->rect();
	QRect sliderRect = rect.adjusted(ellipseR, offsetY, -ellipseR, -offsetY);
	painter.fillRect(sliderRect, Qt::gray);

	//滑块数据
	const  double k = ((double)(value() - minimum())) / (maximum() - minimum());
	int xEllipse = (int)(sliderRect.width() * k) + ellipseR;
	//避免滑块绘制超出边界
	if (xEllipse + ellipseR > rect.width())
	{
		xEllipse -= 1;
	}


	//音量弹簧
	int scaledMag = int(( double)sliderRect.width() * mag + ellipseR);
	int scaledPeak = int(( double)sliderRect.width() * peak + ellipseR);
	//确保scaledMag、scaledPeak不超过滑块的点位
	scaledMag = scaledMag > xEllipse - ellipseR ? xEllipse - ellipseR : scaledMag;
	scaledPeak = scaledPeak > xEllipse - ellipseR ? xEllipse - ellipseR : scaledPeak;

	scaledMag = scaledMag < ellipseR ? ellipseR : scaledMag;
	scaledPeak = scaledPeak < ellipseR ? ellipseR : scaledPeak;

	QLinearGradient gradient;
	gradient.setStart(qreal(scaledMag), 0);
	gradient.setFinalStop(qreal(scaledPeak), 0);
	gradient.setColorAt(0, magColor);
	gradient.setColorAt(1, peakColor);

	// RMS
	painter.fillRect(sliderRect.x(), sliderRect.y(),
		scaledMag, sliderRect.height(),
		magColor);

	// RMS - Peak gradient
	painter.fillRect(scaledMag, sliderRect.y(),
		scaledPeak - scaledMag + 1, sliderRect.height(),
		QBrush(gradient));

	// Background
	painter.fillRect(scaledPeak, sliderRect.y(),
		sliderRect.width() - scaledPeak, sliderRect.height(),
		Qt::gray);

	//最后抗锯齿绘制滑块，抗锯齿会影响性能
	painter.setRenderHint(QPainter::Antialiasing, true);
	QPoint tickCenter(xEllipse, size().height() / 2);
	const QBrush oldBrush = painter.brush();
	const QPen oldPen = painter.pen();
	painter.setBrush(Qt::lightGray);
	painter.setPen(Qt::lightGray);
	painter.drawEllipse(tickCenter, ellipseR, ellipseR);
	painter.setBrush(oldBrush);
	painter.setPen(oldPen);
	painter.setRenderHint(QPainter::Antialiasing, false);
}
