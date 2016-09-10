#pragma once

#include "base/memory/ref_ptrcounted.h"
#include "base/memory/ref_counted.h"
#include <map>
#include "sqlite/sqlite3.h"
#include "db/IPrgDBRecordSet.h"

class CPrgDBRecordSet:public base::RefPtrCounted<prg::IPrgDBRecordSet, base::ThreadSafeRefPolicy>
{
public:
	CPrgDBRecordSet(void);
	virtual ~CPrgDBRecordSet(void);

	HRESULT Next(bool *pNOEOF);
	HRESULT Close();
	HRESULT GetFieldCount(long *pCount);
	HRESULT Index2Name(int index, std::string &name);
	HRESULT Name2Index(const std::string &name, int &index);
	HRESULT GetIntByName(const std::string &name, int &value);
	HRESULT GetInt64ByName(const std::string &name, int64 &value);
	HRESULT GetDoubleByName(const std::string &name, double &value);
	HRESULT GetStringByName(const std::string &name, std::string &value);
	HRESULT GetString16ByName(const std::string &name, string16 &value);
	HRESULT GetIntByIndex(int index, int &value);
	HRESULT GetInt64ByIndex(int index, int64 &value);
	HRESULT GetDoubleByIndex(int index, double &value);
	HRESULT GetStringByIndex(int index, std::string &value);
	HRESULT GetString16ByIndex(int index, string16 &value);

public:
	HRESULT Initialize(sqlite3* pSqlite, sqlite3_stmt* pStmt, bool bSingle, const std::string &sql);

	typedef std::map<std::string,int> NameMap;

private:
	void BuildName2Index();

private:
	sqlite3 *m_pSqlite;
	sqlite3_stmt *m_pStmt;
	bool m_bSingle;
	std::string m_sql;	
	NameMap m_mapName2Index;
};

