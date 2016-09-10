
#pragma once

#include <QMainWindow>
#include <string>
#include <memory>
#include <qwebengineview.h>

#include "login/LoginBiz.h"
#include "ui_LoginWnd.h"

class LoginWnd 
	: public QMainWindow
	, public ILoginCallback
{
	Q_OBJECT

private:
	std::unique_ptr<Ui::LoginWnd> ui;
	bool m_bRealPass;
	bool m_bTryAutoLogin = false;

	QPoint m_clickPos;

public:
	LoginWnd(QWidget *parent = nullptr);
	~LoginWnd();

public:
	void OnLoginSuccess() override;
	void OnLoginFail(PrgString errMsg) override;
	void OnLoginFailWithCode(const char* errCode) override;
	void OnNeed2ndAuth(PrgString phoneNo) override;
	void OnNeedCheckCode(PrgString strCheckCodeUrl) override;
	void OnSecurityGenCodeOk() override;
	void OnTaoBaoTokenOk() override;
	void OnVerifyCodeFail() override;
	void OnTaoBaoTokenFail() override;

protected:
	void keyPressEvent(QKeyEvent *event) override;

	void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;

private slots:
	void on_loginButton_clicked();

	void OnUserSelChanged(const QString & user);
	void OnPsdEdited(const QString &);
	void OnUserEdited(const QString &);

private:
	void DoLogin();
	void ShowErrorMsg(QString errMsg);
};
