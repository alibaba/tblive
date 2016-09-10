//------------------------------------------------------------------------------
//
//    版权所有(C) 阿里巴巴（中国）网络技术有限公司 保留所有权利
//
//    创建者:   徐明
//    创建日期: 2015-1-29
//    功能描述: PrgDBRecordSet的接口声明
//
//
//------------------------------------------------------------------------------

#ifndef __IPrgDBRECORDSET_H_6063515E_090A_446C_B30C_039EE75D0E49_INCLUDED__
#define __IPrgDBRECORDSET_H_6063515E_090A_446C_B30C_039EE75D0E49_INCLUDED__

#include "base/namespace.h"
#include "base/memory/iref_counted.h"
#include "string"
#include "base/files/file_path.h"

BEGIN_NAMESPACE(prg)

class IPrgDBRecordSet:public base::IRefCounted
{
public:
	virtual HRESULT Next(bool *pNOEOF) = 0;
	virtual HRESULT Close() = 0;
	virtual HRESULT GetFieldCount(long *pCount) = 0;
	virtual HRESULT Index2Name(int index, std::string &name) = 0;
	virtual HRESULT Name2Index(const std::string &name, int &index) = 0;

	virtual HRESULT GetInt64ByName(const std::string &name, int64 &value) = 0;
	virtual HRESULT GetIntByName(const std::string &name, int &value) = 0;
	virtual HRESULT GetDoubleByName(const std::string &name, double &value) = 0;
	virtual HRESULT GetStringByName(const std::string &name, std::string &value) = 0;
	virtual HRESULT GetString16ByName(const std::string &name, string16 &value) = 0;

	virtual HRESULT GetInt64ByIndex(int index, int64 &value) = 0;
	virtual HRESULT GetIntByIndex(int index, int &value) = 0;
	virtual HRESULT GetDoubleByIndex(int index, double &value) = 0;
	virtual HRESULT GetStringByIndex(int index, std::string &value) = 0;
	virtual HRESULT GetString16ByIndex(int index, string16 &value) = 0;
};

END_NAMESPACE()

#endif // #ifndef __IPrgDBRECORDSET_H_6063515E_090A_446C_B30C_039EE75D0E49_INCLUDED__
