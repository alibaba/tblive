#pragma once

#include <obs.hpp>
#include <QWidget>
#include <QSlider>
class QPushButton;



class SliderCustom : public QSlider
{
	Q_OBJECT
		Q_PROPERTY(QColor bkColor READ getBkColor WRITE setBkColor DESIGNABLE true)
		Q_PROPERTY(QColor magColor READ getMagColor WRITE setMagColor DESIGNABLE true)
		Q_PROPERTY(QColor peakColor READ getPeakColor WRITE setPeakColor DESIGNABLE true)
		Q_PROPERTY(QColor peakHoldColor READ getPeakHoldColor WRITE setPeakHoldColor DESIGNABLE true)

private:
	 double mag;
	 double peak;
	 double peakHold;

	QColor bkColor, magColor, peakColor, peakHoldColor;
	QTimer *resetTimer;

public:
	explicit SliderCustom(QWidget *parent = Q_NULLPTR);
	explicit SliderCustom(Qt::Orientation orientation, QWidget *parent = Q_NULLPTR);
	void setLevels( double nmag,  double npeak,  double npeakHold);
	QColor getBkColor() const;
	void setBkColor(QColor c);
	QColor getMagColor() const;
	void setMagColor(QColor c);
	QColor getPeakColor() const;
	void setPeakColor(QColor c);
	QColor getPeakHoldColor() const;
	void setPeakHoldColor(QColor c);

protected:
	void paintEvent(QPaintEvent *event);
	private slots:
	void resetState();
};

class QLabel;
class MuteCheckBox;

class VolControlCustom : public QWidget {
	Q_OBJECT

private:
	OBSSource source;
	QLabel          *nameLabel;
	QLabel          *volLabel;
	QLabel          *lineLabel;
	SliderCustom         *slider;
	MuteCheckBox    *mute;
	 double           levelTotal;
	 double           levelCount;
	obs_fader_t     *obs_fader;
	obs_volmeter_t  *obs_volmeter;

	static void OBSVolumeChanged(void *param, float db);
	static void OBSVolumeLevel(void *data, float level, float mag,
		float peak, float muted);
	static void OBSVolumeMuted(void *data, calldata_t *calldata);

	void SetMuteCtrlChecked(bool bMuted);
	void ChangeMuteCtrlToolTip(bool bMuted);
private slots:
	void VolumeChanged();
	void VolumeMuted(bool muted);
	void VolumeLevel( double mag,  double peak,  double peakHold, bool muted);

	void SetMuted(bool checked);
	void SliderChanged(int vol);
	void updateText();

signals:
	void ConfigClicked();

public:
	VolControlCustom(OBSSource source);
	~VolControlCustom();

	inline obs_source_t *GetSource() const {return source;}

	QString GetName() const;
	void SetName(const QString &newName);
};
