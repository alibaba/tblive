
#include "HostApplication.h"

#include <base/file_util.h>

scoped_refptr<CHostApplication> CHostApplication::m_spHost = NULL;

scoped_refptr<CHostApplication> CHostApplication::GetInstance() 
{
    if (m_spHost == NULL)
    {
        m_spHost = new CHostApplication();
    }

    return m_spHost;
}

CHostApplication::CHostApplication(void)
    : m_nSvrType(uauth::ServerType_Release)
{
}


PrgString CHostApplication::GetAppName()
{
    return L"tblive";
}

PrgString CHostApplication::GetAppVersion() 
{
    return L"1.00.00";
}

PrgString CHostApplication::GetUMIDCode()
{
	return L"";
}

base::FilePath CHostApplication::GetDataDir()
{
    return m_dataPath;
}


uauth::ServerType CHostApplication::GetServerType()
{
    return m_nSvrType;
}

void CHostApplication::GetNetProxy( 
    uauth::HTTP_CLIENT_RPOXY_TYPE & type, 
    PrgString& host,
    PrgString& port,
    PrgString& domain,
    PrgString& user,
    PrgString& password )
{

}

void CHostApplication::SetDataDir( const base::FilePath& filPath )
{
    m_dataPath = filPath;
}

void CHostApplication::SetServerType(uauth::ServerType type)
{
    m_nSvrType = type;
}
