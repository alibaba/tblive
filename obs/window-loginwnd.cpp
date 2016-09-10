
#include "window-loginwnd.hpp"
#include "ui_LoginWnd.h"
#include "obs-app.hpp"
#include "tblive_sdk/biz.h"
#include "shell/tblive_cmd_dispatcher.h"

#include "window-checkcodedialog.hpp"
#include "window-need2ndauthdialog.hpp"
#include "login/RecentLoginList.h"

#include <QKeyEvent>
#include <QPushButton>
#include <QMessageBox>
#include <QLabel>
#include <QFontDatabase>

#include "switches.h"
#include "base/command_line.h"
#include "net/base/escape.h"

using namespace std;

LoginWnd::LoginWnd(QWidget *parent)
	: QMainWindow (parent)
	, ui (new Ui::LoginWnd)
	, m_bRealPass(true)
{
	ui->setupUi(this);

	// Set no titlebar
	setWindowFlags(Qt::FramelessWindowHint);

	ui->passwordEdit->setEchoMode(QLineEdit::Password);
	ui->userList->lineEdit()->setPlaceholderText(QTStr("Login.UserNameEditHolder"));

	// Set CheckBox "Auto Login" invisible temporary
	ui->autoLoginCheckBox->setVisible(false);

	// Set "Save Password" checked by default
	ui->savePsdCheckBox->setChecked(true);

	CLoginBiz::GetInstance()->SetLoginCallback(this);

	connect(ui->userList, SIGNAL(currentIndexChanged(const QString &)), 
		this, SLOT(OnUserSelChanged(const QString &)));
	connect(ui->passwordEdit, SIGNAL(textEdited(const QString &)), 
		this, SLOT(OnPsdEdited(const QString &)));
	connect(ui->userList, SIGNAL(editTextChanged(const QString &)),
		this, SLOT(OnUserEdited(const QString &)));

	// Set icon
	QLabel* userIcon = new QLabel(ui->userList);
	userIcon->setObjectName(QStringLiteral("userIcon"));
	userIcon->setGeometry(QRect(8, 5, 16, 16));
	userIcon->setPixmap(QPixmap(QString::fromUtf8(":/res/images/user_icon.png")));
	ui->userList->setStyleSheet("padding: 2 2 2 26;");
	
	ui->passwordEdit->addAction(QIcon(":/res/images/psd_icon.png"), QLineEdit::LeadingPosition);

	// Recent login list
	CRecentLoginList::GetInstance()->GetRecentUserList(
		base::make_lambda_arg_callback< std::list<RecentUserItem> >([this](std::list<RecentUserItem> userList){
		// Get recent login list
		for (auto it : userList)
		{
			ui->userList->addItem(QString::fromStdWString(it.user.GetString()));
		}

		// Preset user name
		std::wstring strValue = CTBLiveShell::GetInstance()->GetCmd(switches::argLoginUser);
		if (!strValue.empty())
		{
			ui->userList->lineEdit()->setText(QString::fromStdWString(strValue));

			// try auto login
			std::wstring strAutoLogin = CTBLiveShell::GetInstance()->GetCmd(switches::argTryAutoLogin);
			if (strAutoLogin == L"1")
			{
				QString name = ui->userList->currentText();
				RecentUserItem userItem = CRecentLoginList::GetInstance()->GetUserItem(name.toStdWString());
				if (!userItem.auto_token.IsEmpty())
				{
					CLoginBiz::GetInstance()->SetLoginParam(ui->savePsdCheckBox->isChecked(), ui->autoLoginCheckBox->isChecked());
					CLoginBiz::GetInstance()->Login(name.toStdWString().c_str(), userItem.auto_token, uauth::Type_Token, L"");
					
					m_bTryAutoLogin = true;
				}
			}
		}

		if (!m_bTryAutoLogin)
		{
			show();
		}
	}));
}

LoginWnd::~LoginWnd()
{
	CLoginBiz::GetInstance()->SetLoginCallback(nullptr);
}

void LoginWnd::on_loginButton_clicked()
{
	DoLogin();
}

void LoginWnd::DoLogin()
{
	ui->loginButton->setEnabled(false);

	CLoginBiz::GetInstance()->SetLoginParam(ui->savePsdCheckBox->isChecked(), ui->autoLoginCheckBox->isChecked());

	QString name = ui->userList->currentText();
	QString psd = ui->passwordEdit->text();
	if ( m_bRealPass )
	{
		CLoginBiz::GetInstance()->Login(name.toStdWString().c_str(), psd.toStdWString().c_str(), uauth::Type_RealPass, L"");
	}
	else
	{
		// token
		RecentUserItem userItem = CRecentLoginList::GetInstance()->GetUserItem(name.toStdWString());
		CLoginBiz::GetInstance()->Login(name.toStdWString().c_str(), userItem.auto_token, uauth::Type_Token, L"");
	}
}

void LoginWnd::OnLoginSuccess()
{
	if (!App()->OBSInit())
	{
		QApplication::quit();
	}

	hide();
	close();
}

void LoginWnd::OnLoginFail(PrgString errMsg)
{
	if ( m_bTryAutoLogin )
	{
		show();
	}

	// 提示错误文案
	if ( errMsg.IsEmpty() )
	{
		ShowErrorMsg(QTStr("Login.ErrorCommon"));
	}
	else
	{
		ShowErrorMsg(QString::fromStdWString(errMsg.GetString()));
	}


	ui->loginButton->setEnabled(true);
}

void LoginWnd::OnLoginFailWithCode(const char* errCode)
{
	if (m_bTryAutoLogin)
	{
		show();
	}

	ShowErrorMsg(QTStr(errCode));

	ui->loginButton->setEnabled(true);
}

void LoginWnd::OnNeed2ndAuth(PrgString phoneNo)
{
	if (m_bTryAutoLogin)
	{
		show();
	}

	Need2ndAuthDialog * dlg = new Need2ndAuthDialog(this);
	dlg->SetPhoneNo(QString::fromStdWString(phoneNo.GetString()));
	bool accepted = (dlg->exec() == QDialog::Accepted);
	if (accepted)
	{
		QString code = dlg->GetSecurityCode();
		CLoginBiz::GetInstance()->DoSecurityAuth(code.toStdWString());
	}
	else
	{
		ShowErrorMsg(QTStr("Login.Need2ndVerifyFailed"));

		ui->loginButton->setEnabled(true);
	}
}

void LoginWnd::OnNeedCheckCode(PrgString strCheckCodeUrl)
{
	if (m_bTryAutoLogin)
	{
		show();
	}

	CheckCodeDialog * dlg = new CheckCodeDialog(this);
	dlg->LoadUrl(QString::fromStdWString(strCheckCodeUrl.GetString()));
	bool accepted = (dlg->exec() == QDialog::Accepted);
	if ( accepted )
	{
		QString name = ui->userList->currentText();
		QString psd = ui->passwordEdit->text();

		QString code = dlg->GetCheckCode();
		if ( m_bRealPass )
		{
			CLoginBiz::GetInstance()->Login(name.toStdWString().c_str(), psd.toStdWString().c_str(), uauth::Type_RealPass, code.toStdWString().c_str());
		}
		else
		{
			RecentUserItem userItem = CRecentLoginList::GetInstance()->GetUserItem(name.toStdWString());
			CLoginBiz::GetInstance()->Login(name.toStdWString().c_str(), userItem.auto_token, uauth::Type_Token, code.toStdWString().c_str());
		}
	}
	else
	{
		// 提示输入验证码
		ShowErrorMsg(QTStr("Login.EnterVerifyFailed"));

		ui->loginButton->setEnabled(true);
	}
}

void LoginWnd::OnSecurityGenCodeOk()
{

}

void LoginWnd::OnTaoBaoTokenOk()
{
	biz::onTaoBaoTokenOk();
}

void LoginWnd::OnVerifyCodeFail()
{

}

void LoginWnd::OnTaoBaoTokenFail()
{

}

void LoginWnd::keyPressEvent(QKeyEvent *event)
{
	if ((event->key() == Qt::Key_Enter) || (event->key() == Qt::Key_Return))
	{
		DoLogin();
		return;
	}

	return QMainWindow::keyPressEvent(event);
}

void LoginWnd::mousePressEvent(QMouseEvent *event)
{
	m_clickPos = event->pos();
}

void LoginWnd::mouseMoveEvent(QMouseEvent *event)
{
	QPoint pt = event->pos();
	if ( pt.y() < 120 )
	{
		move(event->globalPos() - m_clickPos);
	}
}


void LoginWnd::OnUserSelChanged(const QString & user)
{
	RecentUserItem item = CRecentLoginList::GetInstance()->GetUserItem(user.toStdWString());
	if ( item.user == user.toStdWString().c_str() )
	{
		ui->savePsdCheckBox->setChecked(item.save_psd);
		ui->autoLoginCheckBox->setChecked(item.auto_login);

		if ( item.save_psd && !item.auto_token.IsEmpty() )
		{// placeholder
			m_bRealPass = false;
			ui->passwordEdit->setText(QStringLiteral("1234567890"));

			if ( item.auto_login )
			{
				DoLogin();
			}
		}
	}
}

void LoginWnd::OnPsdEdited(const QString &)
{
	m_bRealPass = true;
}

void LoginWnd::OnUserEdited(const QString &)
{
	m_bRealPass = true;
	ui->passwordEdit->setText(QStringLiteral(""));

	OnUserSelChanged(ui->userList->currentText());
}

void LoginWnd::ShowErrorMsg(QString errMsg)
{
	QMessageBox messageBox(this);
	messageBox.setWindowTitle(QTStr("Login.ErrorNotifyTitle"));
	messageBox.setText(errMsg);
	messageBox.exec();
}
