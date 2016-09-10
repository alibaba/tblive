#pragma once

#include <list>
#include <base/event.h>
#include <base/memory/iref_counted.h>
#include <core/PrgString.h>
#include <core/IPrgCOMRefCounted.h>
#include <core/prgcom_helper.h>

#include "TCMSInterface.h"
#include "UAuthConstant.h"
#include "UAuthErrorCode.h"


#ifdef	__UAUTH_EXPORT__
#define UAUTHEXPORT	__declspec(dllexport)
#else	
#define UAUTHEXPORT
#endif	

#ifdef	__cplusplus

#define UAUTH_EXTERN	extern UAUTHEXPORT

#else	// #ifdef __cplusplus

// 想在C里面用用这些函数？想的美
#pragma message( "Please use this file in C++") 
#pragma error

#endif	// #ifdef __cplusplus


DEFINE_CLSID(UnifiedAuthService, "{1C9C6FC8-4046-466C-8A13-FA4D5613F6CE}")

namespace uauth
{
    enum SecurityLoginType
    {
        LoginVerifyCert = 0, //证书验证
        LoginVerifyCode,     //短信验证
    };

    struct ModelAuthOkParam
    {
        PrgString strUsrName;
        PrgString strAutoToken;
        PrgString strBizToken;
		PrgString strMachineId;
		PrgString strMac;
		PrgString strCollinaUA;
        unsigned __int64 n64ServerTime;
    };
    struct ModelNeedCheckCodeParam
    {
        PrgString strCheckCodeUrl;
    };
    struct ModelNeed2ndAuthParam
    {
        PrgString strUsrName;
        PrgString strAutoToken;
        PrgString strAuthData;
        unsigned __int64 n64ServerTime;
    };
    struct ModelSecurityProductParam
    {
        ModelSecurityProductParam()
            : isSuccess(false)
            , phone(false)
            , cert(false)
            , wwtaichi(false)
        {

        }
        bool isSuccess;
        bool phone;
        bool cert;
        bool wwtaichi;
        PrgString phoneNo;
        PrgString order;
        PrgString qiandunparam;
    };
    struct ModelSecurityGenCodeParam
    {
        ModelSecurityGenCodeParam()
            : isSuccess(false)
        {

        }
        bool isSuccess;
        PrgString checkType;
        PrgString message;
    };

    struct ModelTaobaoTokenParam
    {
        ModelTaobaoTokenParam()
            : nExpireTime(0)
        {

        }
        PrgString strToken;
        PrgString strUniqKey;
        UINT nExpireTime;
    };

    struct ModelCertVerifyFailParam
    {
        ModelCertVerifyFailParam() : nErrorCode(0)
        {
        }
        int nErrorCode;
        PrgString strErrorMsg;
    };
    struct ModelCodeVerifyFailParam
    {
        ModelCodeVerifyFailParam() : nErrorCode(0)
        {
        }
        int nErrorCode;
        PrgString strErrorMsg;
    };
    struct ModelAuthFailParam
    {
        ModelAuthFailParam() : nErrorCode(0)
        {
        }

        int nErrorCode;
        PrgString strErrorMsg;
    };
    struct ModelTaoBaoTokenFailParam
    {
        ModelTaoBaoTokenFailParam() : nErrorCode(0)
        {
        }

        int nErrorCode;
        PrgString strErrorMsg;
    };

    struct ModelSendCodeFailParam
    {
        ModelSendCodeFailParam() : nErrorCode(0)
        {
        }

        int nErrorCode;
        PrgString strErrorMsg;
    };


    struct ModelNetDisConnectParam
    {
    };

    class IUnifiedAuthService : public prg::IPrgCOMRefCounted
    {
    public:
        //************************************************************************
        // Description:初始化
        // Returns:    void
        //************************************************************************
        virtual void Initialize(bool bUseTcmsChannel, bool bUseTaobaoSso) = 0;

        //************************************************************************
        // Description:
        // Returns:    void
        //************************************************************************
        virtual void UnInit() = 0;

        //************************************************************************
        // Description:登录
        // Returns:    void
        // Parameter:  const string & strAppid, 
        // Parameter:  const string & strUsrName
        // Parameter:  const string & strToken
        //************************************************************************
        virtual void DoAuth(const PrgString&strUsrName, const PrgString&strPass, LoginType tokenType, 
            const PrgString& strCheckCode) = 0;
        //************************************************************************
        // Description:二次验证
        // Returns:    void
        //************************************************************************
        virtual void DoSecurityAuth(const PrgString&strAuthData, const PrgString &umid, SecurityLoginType type) = 0;        
        //************************************************************************
        // Description:二次验证产品查询
        // Returns:    void
        //************************************************************************
        virtual void DoSecurityMethod() = 0;
        //************************************************************************
        // Description:发送验证码
        // Returns:    void
        //************************************************************************
        virtual void DoSecuritySendCode() = 0;
        //************************************************************************
        // Description:获取loginbyim的token
        // Returns:    void
        //************************************************************************
        virtual void GetTaobaoToken(const PrgString&strUsrName, const PrgString& strBizToken) = 0;


    public:
        base::event<void(ModelAuthOkParam)>        onAuthOk;
        base::event<void(ModelNeedCheckCodeParam)>        onNeedCheckCode;
        base::event<void(ModelNeed2ndAuthParam)>        onNeed2ndAuth;
        base::event<void(ModelSecurityProductParam)>        onSecurityProductOk;
        base::event<void(ModelSecurityGenCodeParam)>        onSecuritySendCodeOk;
        base::event<void(ModelSendCodeFailParam)>           onSecuritySendCodeFail;
        base::event<void(ModelTaobaoTokenParam)>           onTaoBaoTokenOk;
        base::event<void(ModelCertVerifyFailParam)>        onVerifyCertFail;
        base::event<void(ModelCodeVerifyFailParam)>        onVerifyCodeFail;
        base::event<void(ModelTaoBaoTokenFailParam)>       onTaoBaoTokenFail;
        base::event<void(ModelAuthFailParam)>      onAuthFail;
        base::event<void(ModelNetDisConnectParam)>  onNetDisconnect;
    };

    inline scoped_refptr<IUnifiedAuthService> GetUnifiedAuthService(const PrgString &accountId=L"")
    {
        std::string groupId = base::WideToUTF8(accountId.GetString());
        scoped_refptr<IUnifiedAuthService> spInterface;
        prg::PrgCOMGetInstance(c_uuidof(UnifiedAuthService), "", spInterface, "", groupId);
        return spInterface;
    }


	UAUTH_EXTERN tcmslib::ITCMSChannel* GetTCMSChannelInstance();
}

DEFINE_IID(uauth::IUnifiedAuthService, "{7E476BF1-F030-42DC-AAFA-D91187E4B195}")
