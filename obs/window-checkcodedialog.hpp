
#pragma once

#include <QDialog>
#include <string>
#include <memory>

#include "ui_CheckCodeDialog.h"

class CheckCodeDialog : public QDialog {
	Q_OBJECT

private:
	std::unique_ptr<Ui::CheckCodeDialog> ui;

public:
	CheckCodeDialog(QWidget *parent);
	void LoadUrl( QString url );
	QString GetCheckCode();
};
