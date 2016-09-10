//------------------------------------------------------------------------------
//
//    版权所有(C) 阿里巴巴（中国）网络技术有限公司 保留所有权利
//
//    创建者:   徐明
//    创建日期: 2015-1-29
//    功能描述: PrgDBConnect的接口声明
//
//
//------------------------------------------------------------------------------

#ifndef __IPrgDBCONNECT_H_6063515E_090A_446C_B30C_039EE75D0E49_INCLUDED__
#define __IPrgDBCONNECT_H_6063515E_090A_446C_B30C_039EE75D0E49_INCLUDED__

#include "base/namespace.h"
#include "base/memory/iref_counted.h"
#include "string"
#include "base/files/file_path.h"
#include "db/IPrgDBCommand.h"
#include "db/IPrgDBRecordSet.h"

BEGIN_NAMESPACE(prg)

class IPrgDBConnection:public base::IRefCounted
{
public:
	virtual HRESULT Connect(const base::FilePath &dbPath, const std::string &dbKey) = 0;
	virtual HRESULT SuperConnect(const base::FilePath &dbPath, const std::string &dbKey) = 0;
	virtual HRESULT Close() = 0;
	virtual HRESULT CreateCommand(IPrgDBCommand** ppPrgDBCommand) = 0;
	virtual HRESULT ExecuteQuery(const std::string &sql, IPrgDBRecordSet **ppPrgDBRecordset) = 0;
	virtual HRESULT ExecuteNonQuery(const std::string &sql, long *pRecordsAffected, int64 *pLastRowID) = 0;
	virtual HRESULT BeginTransaction(const std::string &name) = 0;
	virtual HRESULT CommitTransaction(const std::string &name) = 0;
	virtual HRESULT RollbackTransaction(const std::string &name) = 0;
};

END_NAMESPACE()

#endif // #ifndef __IPrgDBCONNECT_H_6063515E_090A_446C_B30C_039EE75D0E49_INCLUDED__
