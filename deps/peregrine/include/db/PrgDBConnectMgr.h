#pragma once
#include "base/memory/ref_ptrcounted.h"
#include "base/memory/ref_counted.h"
#include "base/synchronization/lock.h"
#include <map>
#include "db/IPrgDBConnectMgr.h"
#include "base/threading/platform_thread.h"

class CPrgDBConnectMgr:public base::RefPtrCounted<prg::IPrgDBConnectMgr, base::ThreadSafeRefPolicy>
{
public:
	CPrgDBConnectMgr(void);
	virtual ~CPrgDBConnectMgr(void);

	HRESULT InitConnection(const std::string &name, const base::FilePath &dbPath, const std::string &dbKey, const bool superDB = false);
    HRESULT UninitConnection(const std::string &name);
	HRESULT GetConnection(const std::string &name, prg::IPrgDBConnection **ppPrgDBConnection);
	HRESULT CloseCurrentThreadConnection(const std::string &name);
	HRESULT CloseConnection(const std::string &name);
	HRESULT CreateConnection(const std::string &name, prg::IPrgDBConnection **ppPrgDBConnection);

	//============================================================
	HRESULT IsSuperDB(const base::FilePath &dbPath, bool *pSuperDB);
	HRESULT GetSuperDBHead(const base::FilePath &dbPath, const long nLen, unsigned char *pBuf);
	HRESULT UpdateSuperHead(const base::FilePath &dbPath, const unsigned char *pBuf, long nLen);
	//============================================================


private:
	HRESULT CreateConnectionImpl(const base::FilePath &dbPath, const std::string &dbKey, bool superDB, prg::IPrgDBConnection **ppPrgDBConnection);
	bool TestSuperDB(const base::FilePath &dbPath);

	struct DBFileDesc
	{
		std::string name;
		std::string dbkey;
		base::FilePath dbpath;
		bool superdb;
	};

	struct ThreadConnection
	{
		scoped_refptr<prg::IPrgDBConnection> spDBConnection;
		int dwGetConnectionCount;

		ThreadConnection() : dwGetConnectionCount(0)    // 统计数据，被外部使用过的次数
		{
		}
	};

	class ConnectionInfo:public base::RefCountedThreadSafe<ConnectionInfo>
	{
	public:
		// Key : ThreadID
		std::map<base::PlatformThreadId, ThreadConnection> mapThreadId2Connection;

		ConnectionInfo()
		{
		}

		~ConnectionInfo()
		{
		}

		base::Lock mutex;
	};

	scoped_refptr<ConnectionInfo> GetConnectionInfo(const std::string &name, std::string *pDBKey = NULL, base::FilePath *pDBFilePath = NULL, bool *pSuperDB = NULL);

private:
	base::Lock m_lock;
	std::map<std::string, DBFileDesc> m_mapName2DBFileDesc;
	// Key : name
	std::map<std::string, scoped_refptr<ConnectionInfo>> m_mapConnectiones;
};

