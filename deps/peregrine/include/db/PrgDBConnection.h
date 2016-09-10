#pragma once
#include "base/memory/ref_ptrcounted.h"
#include "base/memory/ref_counted.h"

#include "sqlite/sqlite3.h"
#include "db/IPrgDBConnection.h"
#include "db/IPrgDBCommand.h"
#include "db/IPrgDBRecordSet.h"

class CPrgDBConnection:public base::RefPtrCounted<prg::IPrgDBConnection, base::ThreadSafeRefPolicy>
{
public:
	CPrgDBConnection(void);
	virtual ~CPrgDBConnection(void);

	HRESULT Connect(const base::FilePath &dbPath, const std::string &dbKey);
	HRESULT SuperConnect(const base::FilePath &dbPath, const std::string &dbKey);
	HRESULT Close();
	HRESULT CreateCommand(prg::IPrgDBCommand **ppPrgDBCommand);
	HRESULT ExecuteQuery(const std::string &sql, prg::IPrgDBRecordSet **ppPrgDBRecordset);
	HRESULT ExecuteNonQuery(const std::string &sql, long *pRecordsAffected, int64 *pLastRowID);
	HRESULT BeginTransaction(const std::string &name);
	HRESULT CommitTransaction(const std::string &name);
	HRESULT RollbackTransaction(const std::string &name);

private:
	void SetSqliteTimeout(int timeout);

private:
	sqlite3 *m_pSqlite;
};

