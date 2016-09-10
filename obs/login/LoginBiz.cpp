
#include "LoginBiz.h"
#include "HostApplication.h"

#include <base/path_service.h>
#include "base/sys_info.h"
#include "base/base_util.h"

#include <core/prgcom_helper.h>
#include <core/PrgCOMFactory.h>
#include <core/IPrgCOMRefCounted.h>
#include <AuthService/ISDKService.h>
#include <AuthService/ISSOService.h>
#include "AuthService/IUnifiedAuthService.h"

#include "login/RecentLoginList.h"
#include "url/gurl.h"
#include "tblive_sdk/biz.h"

namespace {

const char* SSO_DOMAINS[] = {
	"taobao.com",
	"tmall.com",
	"alipay.com",
	"alitrip.com",
	"aliloan.com"
};

bool CanSSO(PrgString url)
{
	GURL gurl(url.GetString());
	if ( gurl.is_valid() )
	{
		std::string host = gurl.host();
		if ( !host.empty() )
		{
			for ( auto sso_host : SSO_DOMAINS )
			{
				// Need match sso_host backward
				size_t pos = host.find(sso_host);
				if ( pos != std::string::npos && 
					pos+strlen(sso_host) == host.size() )
				{
					return true;
				}
			}
		}
	}

	return false;
}


}// namespace

/*static*/ 
CLoginBiz* CLoginBiz::GetInstance()
{
	return Singleton < CLoginBiz,
		StaticMemorySingletonTraits<CLoginBiz> > ::get();
}

CLoginBiz::CLoginBiz() 
	: m_serverTime(0)
	, m_pLoginCallback(nullptr)
	, m_savePsd(false)
	, m_autoLogin(false)
	, m_bRelogin(false)
{
}

CLoginBiz::~CLoginBiz()
{
}

void CLoginBiz::Init(base::FilePath dataDir)
{
	CHostApplication::GetInstance()->SetDataDir(dataDir);

	base::FilePath exepath;
	PathService::Get(base::FILE_EXE, &exepath);
	prg::PrgCOMInit(exepath.DirName());

	scoped_refptr<uauth::ISDKService> spService;
	PrgCOMGetInstance(c_uuidof(SDKService), "", spService);
	if (spService)
	{
		spService->Init(CHostApplication::GetInstance());
	}

	uauth::GetUnifiedAuthService()->Initialize(false, true);

	uauth::GetUnifiedAuthService()->onAuthOk += base::Bind(&CLoginBiz::OnLoginSuccess, base::Unretained(this));
	uauth::GetUnifiedAuthService()->onAuthFail += base::Bind(&CLoginBiz::OnLoginFail, base::Unretained(this));
	uauth::GetUnifiedAuthService()->onNeed2ndAuth += base::Bind(&CLoginBiz::OnNeed2ndAuth, base::Unretained(this));
	uauth::GetUnifiedAuthService()->onNeedCheckCode += base::Bind(&CLoginBiz::OnNeedCheckCode, base::Unretained(this));
	uauth::GetUnifiedAuthService()->onSecurityProductOk += base::Bind(&CLoginBiz::OnSecurityProductOk, base::Unretained(this));
	uauth::GetUnifiedAuthService()->onSecuritySendCodeOk += base::Bind(&CLoginBiz::OnSecurityGenCodeOk, base::Unretained(this));
	uauth::GetUnifiedAuthService()->onVerifyCodeFail += base::Bind(&CLoginBiz::OnVerifyCodeFail, base::Unretained(this));
	
	uauth::GetUnifiedAuthService()->onTaoBaoTokenOk += base::Bind(&CLoginBiz::OnTaoBaoTokenOk, base::Unretained(this));
}

void CLoginBiz::SetLoginCallback(ILoginCallback* callback)
{
	m_pLoginCallback = callback;
}

void CLoginBiz::SetLoginParam(bool savePsd, bool autoLogin)
{
	m_savePsd = savePsd;
	m_autoLogin = autoLogin;
}

void CLoginBiz::Login(PrgString user, PrgString psd, uauth::LoginType type, PrgString checkCode)
{
	uauth::GetUnifiedAuthService()->DoAuth(user, psd, type, checkCode);
}

void CLoginBiz::DoSecuritySendCode()
{
	uauth::GetUnifiedAuthService()->DoSecuritySendCode();
}

void CLoginBiz::DoSecurityAuth(PrgString securityCode)
{
	std::string umid = base::SysInfo::GetMacInfo();
	uauth::GetUnifiedAuthService()->DoSecurityAuth(securityCode, base::UTF8ToWide(umid), uauth::LoginVerifyCode);
}

void CLoginBiz::OnLoginSuccess(uauth::ModelAuthOkParam param)
{
	m_loginedUser = param.strUsrName;
	m_autoToken = param.strAutoToken;
	m_bizToken = param.strBizToken;
	m_serverTime = param.n64ServerTime;

	uauth::GetUnifiedAuthService()->GetTaobaoToken(m_loginedUser, m_bizToken);

	if (!m_bRelogin)
	{
		// callback
		if ( m_pLoginCallback )
		{
			m_pLoginCallback->OnLoginSuccess();
		}
	}

	// save user and auto token
	RecentUserItem userItem;
	userItem.user = m_loginedUser;
	userItem.save_psd = m_savePsd;
	if ( m_savePsd )
	{
		userItem.auto_token = m_autoToken;
	}
	userItem.auto_login = m_autoLogin;

	CRecentLoginList::GetInstance()->AddUser(userItem);

	// Set timer to relogin after 20min to keep sso
	base::GetUIMessageLoop()->PostDelayedTask(FROM_HERE, base::Lambda([this]() {
		m_bRelogin = true;
		uauth::GetUnifiedAuthService()->DoAuth(m_loginedUser, m_autoToken, uauth::Type_Token, L"");
	}), base::TimeDelta::FromMinutes(20));

	// Check tblive update
#if defined(NDEBUG)
	if (!m_bRelogin)
	{
		base::GetUIMessageLoop()->PostDelayedTask(FROM_HERE, base::Lambda([]() {
			biz::checkTBliveUpdate();
		}), base::TimeDelta::FromSeconds(30));
	}
#endif
}

void CLoginBiz::OnLoginFail(uauth::ModelAuthFailParam param)
{
	if ( m_pLoginCallback )
	{
		m_pLoginCallback->OnLoginFail(param.strErrorMsg);
	}
}

void CLoginBiz::OnNeed2ndAuth(uauth::ModelNeed2ndAuthParam param)
{
	uauth::GetUnifiedAuthService()->DoSecurityMethod();
}

void CLoginBiz::OnNeedCheckCode(uauth::ModelNeedCheckCodeParam param)
{
	if ( m_pLoginCallback )
	{
		m_pLoginCallback->OnNeedCheckCode(param.strCheckCodeUrl);
	}
}

void CLoginBiz::OnSecurityProductOk(uauth::ModelSecurityProductParam param)
{
	if ( param.phone )
	{
		if ( m_pLoginCallback )
		{
			m_pLoginCallback->OnNeed2ndAuth(param.phoneNo);
		}
	}
	else
	{
		// no phone error
		if ( m_pLoginCallback )
		{
			m_pLoginCallback->OnLoginFailWithCode("Login.ErrorNoPhone");
		}
	}
}

void CLoginBiz::OnSecurityGenCodeOk(uauth::ModelSecurityGenCodeParam param)
{

}

void CLoginBiz::OnTaoBaoTokenOk(uauth::ModelTaobaoTokenParam param)
{
	if ( m_pLoginCallback )
	{
		m_pLoginCallback->OnTaoBaoTokenOk();
	}
}

void CLoginBiz::OnVerifyCodeFail(uauth::ModelCodeVerifyFailParam param)
{
	if ( m_pLoginCallback )
	{
		m_pLoginCallback->OnLoginFail(param.strErrorMsg);
	}
}

void CLoginBiz::OnTaoBaoTokenFail(uauth::ModelTaoBaoTokenFailParam param)
{

}

bool CLoginBiz::TranslateUrl(PrgString strUrl, PrgString &strDestUrl)
{
	HRESULT hr = S_OK;
	if (CanSSO(strUrl))
	{
		hr = uauth::GetSSOService()->TranslateUrl(strUrl, strDestUrl);
	}
	else
	{
		strDestUrl = strUrl;
	}
	
	return hr == S_OK;
}

PrgString CLoginBiz::GetLoginedUser()
{
	return m_loginedUser;
}
