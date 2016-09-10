#pragma once
#include "core/PrgString.h"
#include "base/memory/iref_counted.h"
#include "core/PrgString.h"
#include "UAuthConstant.h"

namespace uauth
{

enum ServerType
{
    ServerType_Daily,      // daily环境
    ServerType_PreRelease, // 预发环境
    ServerType_Release,    // 线上环境
};

class IHostApplication : public base::IRefCounted
{
public:
    //************************************************************************
    // Description:获取应用名
    // Returns:    ServerType
    //************************************************************************
    virtual PrgString GetAppName() = 0;
	//************************************************************************
	// Description:获取应该版本号
	// Returns:    prg::PrgString
	//************************************************************************
    virtual PrgString GetAppVersion() = 0;
	//************************************************************************
	// Description:获取数据目录
	// Returns:    base::FilePath
	//************************************************************************
	virtual base::FilePath GetDataDir() = 0;
	
	//************************************************************************
	// Description:获取umid
	// Returns:    PrgString
	//************************************************************************
	virtual PrgString GetUMIDCode() = 0;
    //************************************************************************
	// Description:获取运行环境
	// Returns:    ServerType
	//************************************************************************
    virtual ServerType GetServerType() = 0;

    virtual void GetNetProxy( 
        uauth::HTTP_CLIENT_RPOXY_TYPE & type, 
        PrgString& host,
        PrgString& port,
        PrgString& domain,
        PrgString& user,
        PrgString& password ) = 0;
};
}