//------------------------------------------------------------------------------
//
//    版权所有(C) 阿里巴巴（中国）网络技术有限公司 保留所有权利
//
//    创建者:   徐明
//    创建日期: 2015-03-04
//    功能描述: db的c方法封装。
//
//
//------------------------------------------------------------------------------

#ifndef __PrgDB_HELPER_H_75895926_20F7_40B4_A1F4_718D2F4AFFA1_INCLUDED__
#define __PrgDB_HELPER_H_75895926_20F7_40B4_A1F4_718D2F4AFFA1_INCLUDED__
#include "db/IPrgDBConnectMgr.h"
#include "db/PrgDB.h"

BEGIN_NAMESPACE(prg)

inline HRESULT PrgDBInitConnection(const std::string &name, const base::FilePath &dbPath, const std::string &dbKey, const bool superDB = false)
{
    scoped_refptr<IPrgDBConnectMgr> spPrgDBConnectMgr;
	HRESULT hr = GetPrgDBConnectMgr(spPrgDBConnectMgr.getaddress());
	if (SUCCEEDED(hr))
	{
		hr = spPrgDBConnectMgr->InitConnection(name, dbPath, dbKey, superDB);
	}
	return hr;
}

inline HRESULT PrgDBUninitConnection(const std::string &name)
{
    scoped_refptr<IPrgDBConnectMgr> spPrgDBConnectMgr;
    HRESULT hr = GetPrgDBConnectMgr(spPrgDBConnectMgr.getaddress());
    if (SUCCEEDED(hr))
    {
        hr = spPrgDBConnectMgr->UninitConnection(name);
    }
    return hr;
}

inline HRESULT PrgDBCreateConnection(const std::string &name, prg::IPrgDBConnection **ppPrgDBConnection)
{
	scoped_refptr<IPrgDBConnectMgr> spPrgDBConnectMgr;
	HRESULT hr = GetPrgDBConnectMgr(spPrgDBConnectMgr.getaddress());
	if (SUCCEEDED(hr))
	{
		hr = spPrgDBConnectMgr->CreateConnection(name, ppPrgDBConnection);
	}
	return hr;
}

inline HRESULT PrgDBGetConnection(const std::string &name, prg::IPrgDBConnection **ppPrgDBConnection)
{
	scoped_refptr<IPrgDBConnectMgr> spPrgDBConnectMgr;
	HRESULT hr = GetPrgDBConnectMgr(spPrgDBConnectMgr.getaddress());
	if (SUCCEEDED(hr))
	{
		hr = spPrgDBConnectMgr->GetConnection(name, ppPrgDBConnection);
	}
	return hr;
}

inline HRESULT PrgDBCloseCurrentThreadConnection(const std::string &name)
{
	scoped_refptr<IPrgDBConnectMgr> spPrgDBConnectMgr;
	HRESULT hr = GetPrgDBConnectMgr(spPrgDBConnectMgr.getaddress());
	if (SUCCEEDED(hr))
	{
		hr = spPrgDBConnectMgr->CloseCurrentThreadConnection(name);
	}
	return hr;
}

inline HRESULT PrgDBCloseConnection(const std::string &name)
{
	scoped_refptr<IPrgDBConnectMgr> spPrgDBConnectMgr;
	HRESULT hr = GetPrgDBConnectMgr(spPrgDBConnectMgr.getaddress());
	if (SUCCEEDED(hr))
	{
		hr = spPrgDBConnectMgr->CloseConnection(name);
	}
	return hr;
}

inline HRESULT PrgDBIsSuperDB(const base::FilePath &dbPath, bool *pSuperDB)
{
	scoped_refptr<IPrgDBConnectMgr> spPrgDBConnectMgr;
	HRESULT hr = GetPrgDBConnectMgr(spPrgDBConnectMgr.getaddress());
	if (SUCCEEDED(hr))
	{
		hr = spPrgDBConnectMgr->IsSuperDB(dbPath, pSuperDB);
	}
	return hr;
}

inline HRESULT PrgDBGetSuperDBHead(const base::FilePath &dbPath, const long nLen, unsigned char *pBuf)
{
	scoped_refptr<IPrgDBConnectMgr> spPrgDBConnectMgr;
	HRESULT hr = GetPrgDBConnectMgr(spPrgDBConnectMgr.getaddress());
	if (SUCCEEDED(hr))
	{
		hr = spPrgDBConnectMgr->GetSuperDBHead(dbPath, nLen, pBuf);
	}
	return hr;
}

inline HRESULT PrgDBUpdateSuperHead(const base::FilePath &dbPath, const unsigned char *pBuf, long nLen)
{
	scoped_refptr<IPrgDBConnectMgr> spPrgDBConnectMgr;
	HRESULT hr = GetPrgDBConnectMgr(spPrgDBConnectMgr.getaddress());
	if (SUCCEEDED(hr))
	{
		hr = spPrgDBConnectMgr->UpdateSuperHead(dbPath, pBuf, nLen);
	}
	return hr;
}

END_NAMESPACE()

#define BEGIN_CONN_QUERY_TRANS(dbname, sql, hrThrow, spPrgDBRecordSet) 	\
	scoped_refptr<IPrgDBConnection> spPrgDBConnection = NULL; \
	try \
	{ \
		hrThrow = prg::PrgDBGetConnection(dbname, spPrgDBConnection.getaddress()); \
		hrThrow = spPrgDBConnection->BeginTransaction(""); \
		hrThrow = spPrgDBConnection->ExecuteQuery(sql, spPrgDBRecordSet.getaddress()); \

#define END_CONN_QUERY_TRANS(spPrgDBRecordSet) \
		if (spPrgDBRecordSet != NULL) \
		{ \
			spPrgDBRecordSet->Close(); \
			spPrgDBRecordSet = NULL; \
		} \
		if (spPrgDBConnection != NULL) \
		{ \
			spPrgDBConnection->CommitTransaction(""); \
			spPrgDBConnection = NULL; \
		} \
	} \
	catch (_hresult_error& e) \
	{ \
		e; \
		if (spPrgDBRecordSet != NULL) \
		{ \
			spPrgDBRecordSet->Close(); \
			spPrgDBRecordSet = NULL; \
		} \
		if (spPrgDBConnection != NULL) \
		{ \
			spPrgDBConnection->RollbackTransaction(""); \
			spPrgDBConnection = NULL; \
		} \
	};

#define BEGIN_CONN_QUERY_NOTRANS(dbname, sql, hrThrow, spPrgDBRecordSet) 	\
	scoped_refptr<IPrgDBConnection> spPrgDBConnection = NULL; \
	try \
	{ \
		hrThrow = prg::PrgDBGetConnection(dbname, spPrgDBConnection.getaddress()); \
		hrThrow = spPrgDBConnection->ExecuteQuery(sql, spPrgDBRecordSet.getaddress()); \

#define END_CONN_QUERY_NOTRANS(spPrgDBRecordSet) \
		if (spPrgDBRecordSet != NULL) \
		{ \
			spPrgDBRecordSet->Close(); \
			spPrgDBRecordSet = NULL; \
		} \
		if (spPrgDBConnection != NULL) \
		{ \
			spPrgDBConnection = NULL; \
		} \
	} \
	catch (_hresult_error& e) \
	{ \
		e; \
		if (spPrgDBRecordSet != NULL) \
		{ \
			spPrgDBRecordSet->Close(); \
		} \
		if (spPrgDBConnection != NULL) \
		{ \
			spPrgDBConnection = NULL; \
		} \
	};

#define BEGIN_CONN_NOQUERY_TRANS(dbname, hrThrow, spPrgDBConnection) 	\
	try \
	{ \
		hrThrow = prg::PrgDBGetConnection(dbname, spPrgDBConnection.getaddress()); \
		hrThrow = spPrgDBConnection->BeginTransaction(""); \

#define END_CONN_NOQUERY_TRANS(spPrgDBConnection) \
		if (spPrgDBConnection != NULL) \
		{ \
			spPrgDBConnection->CommitTransaction(""); \
			spPrgDBConnection = NULL; \
		} \
	} \
	catch (_hresult_error& e) \
	{ \
		e; \
		if (spPrgDBConnection != NULL) \
		{ \
			spPrgDBConnection->RollbackTransaction(""); \
			spPrgDBConnection = NULL; \
		} \
	};

#define BEGIN_CONN_NOQUERY_NOTRANS(dbname, hrThrow, spPrgDBConnection) 	\
	try \
	{ \
		hrThrow = prg::PrgDBGetConnection(dbname, spPrgDBConnection.getaddress()); \

#define END_CONN_NOQUERY_NOTRANS(spPrgDBConnection) \
		if (spPrgDBConnection != NULL) \
		{ \
			spPrgDBConnection = NULL; \
		} \
	} \
	catch (_hresult_error& e) \
	{ \
		e; \
		if (spPrgDBConnection != NULL) \
		{ \
			spPrgDBConnection = NULL; \
		} \
	};

#define BEGIN_COMMAND_QUERY_TRANS(dbname, sql, hrThrow, spPrgDBRecordSet, spPrgDBCommand) 	\
	scoped_refptr<IPrgDBConnection> spPrgDBConnection = NULL; \
	try \
	{ \
		hrThrow = prg::PrgDBGetConnection(dbname, spPrgDBConnection.getaddress()); \
		hrThrow = spPrgDBConnection->BeginTransaction(""); \
		hrThrow = spPrgDBConnection->CreateCommand(spPrgDBCommand.getaddress()); \
		hrThrow = spPrgDBCommand->PrepareCommand(sql); \

#define END_COMMAND_QUERY_TRANS(spPrgDBRecordSet, spPrgDBCommand) \
		if (spPrgDBRecordSet != NULL) \
		{ \
			spPrgDBRecordSet->Close(); \
			spPrgDBRecordSet = NULL; \
		} \
		if (spPrgDBCommand != NULL) \
		{ \
			spPrgDBCommand->Close(); \
			spPrgDBCommand = NULL; \
		} \
		if (spPrgDBConnection != NULL) \
		{ \
			spPrgDBConnection->CommitTransaction(""); \
			spPrgDBConnection = NULL; \
		} \
	} \
	catch (_hresult_error& e) \
	{ \
		e; \
		if (spPrgDBRecordSet != NULL) \
		{ \
			spPrgDBRecordSet->Close(); \
			spPrgDBRecordSet = NULL; \
		} \
		if (spPrgDBCommand != NULL) \
		{ \
			spPrgDBCommand->Close(); \
			spPrgDBCommand = NULL; \
		} \
		if (spPrgDBConnection != NULL) \
		{ \
			spPrgDBConnection->RollbackTransaction(""); \
			spPrgDBConnection = NULL; \
		} \
	};

#define BEGIN_COMMAND_QUERY_NOTRANS(dbname, sql, hrThrow, spPrgDBRecordSet, spPrgDBCommand) 	\
	scoped_refptr<IPrgDBConnection> spPrgDBConnection = NULL; \
	try \
	{ \
		hrThrow = prg::PrgDBGetConnection(dbname, spPrgDBConnection.getaddress()); \
		hrThrow = spPrgDBConnection->CreateCommand(spPrgDBCommand.getaddress()); \
		hrThrow = spPrgDBCommand->PrepareCommand(sql); \

#define END_COMMAND_QUERY_NOTRANS(spPrgDBRecordSet, spPrgDBCommand) \
		if (spPrgDBRecordSet != NULL) \
		{ \
			spPrgDBRecordSet->Close(); \
			spPrgDBRecordSet = NULL; \
		} \
		if (spPrgDBCommand != NULL) \
		{ \
			spPrgDBCommand->Close(); \
			spPrgDBCommand = NULL; \
		} \
		if (spPrgDBConnection != NULL) \
		{ \
			spPrgDBConnection = NULL; \
		} \
	} \
	catch (_hresult_error& e) \
	{ \
		e; \
		if (spPrgDBRecordSet != NULL) \
		{ \
			spPrgDBRecordSet->Close(); \
			spPrgDBRecordSet = NULL; \
		} \
		if (spPrgDBCommand != NULL) \
		{ \
			spPrgDBCommand->Close(); \
			spPrgDBCommand = NULL; \
		} \
		if (spPrgDBConnection != NULL) \
		{ \
			spPrgDBConnection = NULL; \
		} \
	};

#define BEGIN_COMMAND_NOQUERY_TRANS(dbname, sql, hrThrow, spPrgDBCommand) 	\
	scoped_refptr<IPrgDBConnection> spPrgDBConnection = NULL; \
	try \
	{ \
		hrThrow = prg::PrgDBGetConnection(dbname, spPrgDBConnection.getaddress()); \
		hrThrow = spPrgDBConnection->BeginTransaction(""); \
		hrThrow = spPrgDBConnection->CreateCommand(spPrgDBCommand.getaddress()); \
		hrThrow = spPrgDBCommand->PrepareCommand(sql); \

#define END_COMMAND_NOQUERY_TRANS(spPrgDBCommand) \
		if (spPrgDBCommand != NULL) \
		{ \
			spPrgDBCommand->Close(); \
			spPrgDBCommand = NULL; \
		} \
		if (spPrgDBConnection != NULL) \
		{ \
			spPrgDBConnection->CommitTransaction(""); \
			spPrgDBConnection = NULL; \
		} \
	} \
	catch (_hresult_error& e) \
	{ \
		e; \
		if (spPrgDBCommand != NULL) \
		{ \
			spPrgDBCommand->Close(); \
			spPrgDBCommand = NULL; \
		} \
		if (spPrgDBConnection != NULL) \
		{ \
			spPrgDBConnection->RollbackTransaction(""); \
			spPrgDBConnection = NULL; \
		} \
	};

#define BEGIN_COMMAND_NOQUERY_NOTRANS(dbname, sql, hrThrow, spPrgDBCommand) 	\
	scoped_refptr<IPrgDBConnection> spPrgDBConnection = NULL; \
	try \
	{ \
		hrThrow = prg::PrgDBGetConnection(dbname, spPrgDBConnection.getaddress()); \
		hrThrow = spPrgDBConnection->CreateCommand(spPrgDBCommand.getaddress()); \
		hrThrow = spPrgDBCommand->PrepareCommand(sql); \

#define END_COMMAND_NOQUERY_NOTRANS(spPrgDBCommand) \
		if (spPrgDBCommand != NULL) \
		{ \
			spPrgDBCommand->Close(); \
			spPrgDBCommand = NULL; \
		} \
		if (spPrgDBConnection != NULL) \
		{ \
			spPrgDBConnection = NULL; \
		} \
	} \
	catch (_hresult_error& e) \
	{ \
		e; \
		if (spPrgDBCommand != NULL) \
		{ \
			spPrgDBCommand->Close(); \
			spPrgDBCommand = NULL; \
		} \
		if (spPrgDBConnection != NULL) \
		{ \
			spPrgDBConnection = NULL; \
		} \
	};

#endif // #ifndef __PrgDB_HELPER_H_75895926_20F7_40B4_A1F4_718D2F4AFFA1_INCLUDED__
