#pragma once

#include <core/PrgString.h>
#include <core/prgcom_helper.h>
#include <core/IPrgCOMRefCounted.h>
#include <base/memory/ref_counted.h>


DEFINE_CLSID(SSOService, "{F52BEF9D-CA51-4648-86B8-30A937B5A4D0}")

 namespace uauth
{
    class ISSOService:public prg::IPrgCOMRefCounted
    {
    public:
        virtual HRESULT TranslateUrl(const PrgString& strUrl, PrgString &strDestUrl) = 0;
        virtual HRESULT UpdateSSO(const PrgString& strUserName, const PrgString& strKey, PrgString &strWebPass) = 0;
    };

    inline scoped_refptr<ISSOService> GetSSOService()
    {
        scoped_refptr<ISSOService> spInterface;
        prg::PrgCOMGetInstance(c_uuidof(SSOService), "", spInterface);
        return spInterface;
    }
}

DEFINE_IID(uauth::ISSOService, "{CACA972D-7187-4A49-A7B0-4F6C83C10F72}")
