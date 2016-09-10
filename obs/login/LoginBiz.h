#pragma once

#include <base/memory/ref_ptrcounted.h>
#include <base/memory/singleton.h>
#include "core/PrgString.h"

#include "AuthService/IUnifiedAuthService.h"


struct ILoginCallback
{
	virtual void OnLoginSuccess() = 0;
	virtual void OnLoginFail(PrgString errMsg) = 0;
	virtual void OnLoginFailWithCode(const char* errCode) = 0;
	virtual void OnNeed2ndAuth(PrgString phoneNo) = 0;
	virtual void OnNeedCheckCode(PrgString strCheckCodeUrl) = 0;
	virtual void OnSecurityGenCodeOk() = 0;
	virtual void OnTaoBaoTokenOk() = 0;
	virtual void OnVerifyCodeFail() = 0;
	virtual void OnTaoBaoTokenFail() = 0;
};

class CLoginBiz
{
public:
	CLoginBiz();
	~CLoginBiz();

	static CLoginBiz* GetInstance();

	void Init(base::FilePath dataDir);
	void SetLoginCallback(ILoginCallback* callback);

	void SetLoginParam( bool savePsd, bool autoLogin );
	void Login(PrgString user, PrgString psd, uauth::LoginType type, PrgString checkCode);
	void DoSecurityAuth(PrgString securityCode);

	bool TranslateUrl(PrgString strUrl, PrgString &strDestUrl);
	PrgString GetLoginedUser();
	void DoSecuritySendCode();

private:
	// login
	void OnLoginSuccess(uauth::ModelAuthOkParam param);
	void OnLoginFail(uauth::ModelAuthFailParam param);
	void OnNeed2ndAuth(uauth::ModelNeed2ndAuthParam param);
	void OnNeedCheckCode(uauth::ModelNeedCheckCodeParam param);
	void OnSecurityProductOk(uauth::ModelSecurityProductParam param);
	void OnSecurityGenCodeOk(uauth::ModelSecurityGenCodeParam param);
	void OnTaoBaoTokenOk(uauth::ModelTaobaoTokenParam param);
	void OnVerifyCodeFail(uauth::ModelCodeVerifyFailParam param);
	void OnTaoBaoTokenFail(uauth::ModelTaoBaoTokenFailParam param);

    
private:
	friend struct StaticMemorySingletonTraits <CLoginBiz>;
	DISALLOW_COPY_AND_ASSIGN(CLoginBiz);

	ILoginCallback* m_pLoginCallback;

	PrgString m_loginedUser;
	PrgString m_autoToken;
	PrgString m_bizToken;
	uint64 m_serverTime;
	bool m_savePsd;
	bool m_autoLogin;

	bool m_bRelogin;
};
