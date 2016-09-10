//------------------------------------------------------------------------------
//
//    版权所有(C) 阿里巴巴（中国）网络技术有限公司 保留所有权利
//
//    创建者:   徐明
//    创建日期: 2015-1-29
//    功能描述: PrgDBCommand的接口声明
//
//
//------------------------------------------------------------------------------

#ifndef __IPrgDBCOMMAND_H_6063515E_090A_446C_B30C_039EE75D0E49_INCLUDED__
#define __IPrgDBCOMMAND_H_6063515E_090A_446C_B30C_039EE75D0E49_INCLUDED__

#include "base/namespace.h"
#include "base/memory/iref_counted.h"
#include "string"
#include "base/files/file_path.h"
#include "db/IPrgDBRecordSet.h"
#include "base/strings/string16.h"

BEGIN_NAMESPACE(prg)

class IPrgDBCommand:public base::IRefCounted
{
public:
	virtual HRESULT PrepareCommand(const std::string &sql) = 0;

	virtual HRESULT BindIntParameterByName(const std::string &name, const int val) = 0;
	virtual HRESULT BindInt64ParameterByName(const std::string &name, const int64 val) = 0;
	virtual HRESULT BindDoubleParameterByName(const std::string &name, const double val) = 0;
	virtual HRESULT BindStringParameterByName(const std::string &name, const std::string &val) = 0;
	virtual HRESULT BindString16ParameterByName(const std::string &name, const string16 &val) = 0;

	virtual HRESULT BindIntParameterByIndex(const int index, const int val) = 0;
	virtual HRESULT BindInt64ParameterByIndex(const int index, const int64 val) = 0;
	virtual HRESULT BindDoubleParameterByIndex(const int index, const double val) = 0;
	virtual HRESULT BindStringParameterByIndex(const int index, const std::string &val) = 0;
	virtual HRESULT BindString16ParameterByIndex(const int index, const string16 &val) = 0;

	virtual HRESULT ExecuteQuery(prg::IPrgDBRecordSet **ppPrgDBRecordset) = 0;
	virtual HRESULT ExecuteNonQuery(long* pRecordsAffected, int64* pLastRowID) = 0;
	virtual HRESULT Reset() = 0;
	virtual HRESULT Close() = 0;
};

END_NAMESPACE()

#endif // #ifndef __IPrgDBCOMMAND_H_6063515E_090A_446C_B30C_039EE75D0E49_INCLUDED__
