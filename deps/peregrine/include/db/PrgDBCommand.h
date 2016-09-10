#pragma once
#include "base/memory/ref_ptrcounted.h"
#include "base/memory/ref_counted.h"
#include "sqlite/sqlite3.h"
#include "db/IPrgDBCommand.h"
#include "db/IPrgDBRecordSet.h"

class CPrgDBCommand:public base::RefPtrCounted<prg::IPrgDBCommand, base::ThreadSafeRefPolicy>
{
public:
	CPrgDBCommand(void);
	virtual ~CPrgDBCommand(void);

	HRESULT PrepareCommand(const std::string &sql);
	HRESULT BindIntParameterByName(const std::string &name, const int val);
	HRESULT BindInt64ParameterByName(const std::string &name, const int64 val);
	HRESULT BindDoubleParameterByName(const std::string &name, const double val);
	HRESULT BindStringParameterByName(const std::string &name, const std::string &val);
	HRESULT BindString16ParameterByName(const std::string &name, const string16 &val);
	HRESULT BindIntParameterByIndex(const int index, const int val);
	HRESULT BindInt64ParameterByIndex(const int index, const int64 val);
	HRESULT BindDoubleParameterByIndex(const int index, const double val);
	HRESULT BindStringParameterByIndex(const int index, const std::string &val);
	HRESULT BindString16ParameterByIndex(const int index, const string16 &val);
	HRESULT ExecuteQuery(prg::IPrgDBRecordSet **ppPrgDBRecordset);
	HRESULT ExecuteNonQuery(long* pRecordsAffected, int64* pLastRowID);
	HRESULT Reset();
	HRESULT Close();

public:
	HRESULT Initialize(sqlite3 *m_pSqlite);

private:
	int CloseStmt();

private:
	sqlite3 *m_pSqlite;
	sqlite3_stmt *m_pStmt;
	std::string m_sql;
};

