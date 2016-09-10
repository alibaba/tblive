#pragma once

#include <core/PrgString.h>
#include <core/prgcom_helper.h>
#include <core/IPrgCOMRefCounted.h>
#include <base/memory/ref_counted.h>
#include <base/callback.h>


#include "IHostApplication.h"

DEFINE_CLSID(SDKService, "{E60A96FA-BFE1-46E2-9BF6-3371700264F0}")

namespace uauth
{
    typedef base::Callback<void()> ModelIMServiceUnitedCallback;
    typedef base::Callback<void()> ModelIMServiceUnitedUserCallback;

    class ISDKService:public prg::IPrgCOMRefCounted
    {
    public:
        virtual HRESULT Init(scoped_refptr<IHostApplication> spHostApplication) = 0;
        virtual HRESULT Uninit(ModelIMServiceUnitedCallback callback) = 0;
    };

    inline scoped_refptr<ISDKService> GetSDKService()
    {
        scoped_refptr<ISDKService> spInterface;
        prg::PrgCOMGetInstance(c_uuidof(SDKService), "", spInterface);
        return spInterface;
    }
}

DEFINE_IID(uauth::ISDKService, "{89247078-F152-4D4F-A9EF-CF0C792ED1AE}")
