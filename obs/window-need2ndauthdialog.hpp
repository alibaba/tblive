#pragma once

#include <QDialog>
#include <string>
#include <memory>

#include "ui_Need2ndAuthDialog.h"

class Need2ndAuthDialog : public QDialog {
	Q_OBJECT

private:
	std::unique_ptr<Ui::Need2ndAuthDialog> ui;

public:
	Need2ndAuthDialog(QWidget *parent);
	void SetPhoneNo(QString phoneNo);
	QString GetSecurityCode();


private slots:
	void on_genCodeButton_clicked();
};
