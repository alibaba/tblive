#pragma once

#include <base/memory/ref_ptrcounted.h>

#include "AuthService/IHostApplication.h"

class CHostApplication : public base::RefPtrCounted<uauth::IHostApplication, base::ThreadSafeRefPolicy>
{
public:
    static scoped_refptr<CHostApplication> GetInstance();


public:
    virtual PrgString GetAppName();

    virtual PrgString GetAppVersion();

	virtual PrgString GetUMIDCode();

    virtual base::FilePath GetDataDir();

    virtual uauth::ServerType GetServerType();

    virtual void GetNetProxy( 
        uauth::HTTP_CLIENT_RPOXY_TYPE & type, 
        PrgString& host,
        PrgString& port,
        PrgString& domain,
        PrgString& user,
        PrgString& password );

public:
    void SetDataDir(const base::FilePath& filPath);
    void SetServerType(uauth::ServerType type);

private:
    CHostApplication(void);

private:
    base::FilePath m_dataPath;
    static scoped_refptr<CHostApplication> m_spHost;
    uauth::ServerType m_nSvrType;
};
