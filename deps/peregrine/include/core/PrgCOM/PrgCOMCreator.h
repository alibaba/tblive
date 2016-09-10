//------------------------------------------------------------------------------
//
//    版权所有(C) 阿里巴巴（中国）网络技术有限公司 保留所有权利
//
//    创建者:   徐明
//    创建日期: 2015-02-02
//    功能描述: PrgCOM的创建器，本身包含一份可以创建的COM列表
//
//
//------------------------------------------------------------------------------

#ifndef __PrgCOMCreator_H_BF4263D7_26A5_4973_AC16_8CBAC4BF419A_INCLUDED__
#define __PrgCOMCreator_H_BF4263D7_26A5_4973_AC16_8CBAC4BF419A_INCLUDED__

#include "PrgCOMScaner.h"
#include "map"
#include "base/synchronization/lock.h"
#include "PrgCOMModuleLoader.h"
#include "core/IPrgCOM.h"

class CPrgCOMCreator
{
public:
    CPrgCOMCreator();
    ~CPrgCOMCreator();

    HRESULT Init(const base::FilePath &path);
	HRESULT InitCustomPrgCOMObjectConfig(const std::string &configid, const base::FilePath &userPath);
    HRESULT Uninit();
	HRESULT CanCreateInstance(const std::string &clsid, bool *bCan, const std::string &configid = "");
    HRESULT CreateInstance(const std::string &configid, const std::string &clsid, const std::string &iid, void **ppObject);

	HRESULT GetPrgCOMObjectConfig(const std::string &configid, std::list<scoped_refptr<prg::CPrgCOMObjectIdentify>> &listPrgCOMObject);
	HRESULT EnableCustomPrgCOMObjectConfig(const std::string &configid, const std::string &clsid, bool enable = true);
	HRESULT ResetCustomPrgCOMObjectConfig(const std::string &configid);

private:
	void Reset();
	bool IsClsIdExist(const std::string &clsid);
	bool IsPrgCOMObjectRecursionEnable(const std::string &configid, const std::string &clsid);
	bool IsPrgCOMObjectEnable(const std::string &configid, const std::string &clsid);

private:
  base::Lock m_lock;

	std::map<std::string, COMObjectesIdentify> m_mapClsId2Identify;
	PlatformDisableCOMObjectes m_platformDiable;
	std::map<std::string, UserDisableCOMObjectes> m_userDisables;
	std::map<std::string, base::FilePath> m_userPathes;
	CPrgCOMModuleLoader m_moduleLoader;
};

#endif // #ifndef __PrgCOMCreator_H_BF4263D7_26A5_4973_AC16_8CBAC4BF419A_INCLUDED__
