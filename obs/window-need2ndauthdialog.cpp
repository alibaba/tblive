
#include "window-need2ndauthdialog.hpp"
#include "obs-app.hpp"
#include "login/LoginBiz.h"

using namespace std;

Need2ndAuthDialog::Need2ndAuthDialog(QWidget *parent)
	: QDialog(parent, Qt::WindowCloseButtonHint),
	  ui(new Ui::Need2ndAuthDialog)
{
	ui->setupUi(this);
}

void Need2ndAuthDialog::SetPhoneNo(QString phoneNo)
{
	ui->phoneNoLabel->setText(phoneNo);
}

void Need2ndAuthDialog::on_genCodeButton_clicked()
{
	CLoginBiz::GetInstance()->DoSecuritySendCode();
}

QString Need2ndAuthDialog::GetSecurityCode()
{
	return ui->lineEdit->text();
}
