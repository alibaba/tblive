
#pragma once

#include <QDockWidget>
#include <string>
#include <memory>

#include "ui_HoverWidget.h"
#include "obs.h"

#define WASAPI_INPUT_CAPTURE      "wasapi_input_capture"
#define WASAPI_OUTPUT_CAPTURE     "wasapi_output_capture"

//TODO:
//浮层窗口是通用的需求，可以封装一个HoverWidgetBase基础类，
//让子类继承实现ImplInstallEventFilter(),AddWidget2Layout()接口
class VolHoverWidget : public QDockWidget {
	Q_OBJECT

private:
	std::unique_ptr<Ui::HoverWidget> ui;

public:
	VolHoverWidget(QWidget *parent);

	void ShowAt(int px, int py);

	void AddWidget2Layout(QWidget *w);
	
protected:
	bool eventFilter(QObject *target, QEvent *event);

private:
	void RecheckInputOutputAudioSrc();
	void ImplInstallEventFilter();
};
