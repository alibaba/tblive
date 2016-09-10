//------------------------------------------------------------------------------
//
//    版权所有(C) 阿里巴巴（中国）网络技术有限公司 保留所有权利
//
//    创建者:   徐明
//    创建日期: 2015-02-03
//    功能描述: PrgCOM的路径扫描器，扫描出指定目录底下的COM集合及依赖关系等
//
//
//------------------------------------------------------------------------------

#ifndef __PrgCOMScaner_H_C9362A6A_BBF8_4683_8517_FCC330077845_INCLUDED__
#define __PrgCOMScaner_H_C9362A6A_BBF8_4683_8517_FCC330077845_INCLUDED__

#include "list"
#include "base/files/file_path.h"
#include <base/xparam/xparam_helper.h>

//MAC或者Windows等平台不需要加载的业务模块集合
class PlatformDisableCOMObjectes
{
public:
    PlatformDisableCOMObjectes();
    ~PlatformDisableCOMObjectes();
    
	std::list<std::string> disableCOMObjectes;

	SL_CLASS_MAPPING_BEGIN(PlatformDisableCOMObjectes)
		SL_MEMBER_V(disableCOMObjectes);
	SL_CLASS_MAPPING_END()
};
inline PlatformDisableCOMObjectes::PlatformDisableCOMObjectes(){}
inline PlatformDisableCOMObjectes::~PlatformDisableCOMObjectes(){}

//用户自定义关闭的业务模块集合
class UserDisableCOMObjectes
{
public:
    UserDisableCOMObjectes();
    ~UserDisableCOMObjectes();
	std::list<std::string> disableCOMObjectes;

	SL_CLASS_MAPPING_BEGIN(UserDisableCOMObjectes)
		SL_MEMBER_V(disableCOMObjectes);
	SL_CLASS_MAPPING_END()
};
inline UserDisableCOMObjectes::UserDisableCOMObjectes(){}
inline UserDisableCOMObjectes::~UserDisableCOMObjectes(){}

//业务模块静态依赖表，其中dependency只有一级依赖
class COMObjectesIdentify
{
public:
    COMObjectesIdentify();
    ~COMObjectesIdentify();
    
	std::string clsid;
	std::string name;
	std::string desc;
	std::string dllname;
	std::string dllpath;
	std::list<COMObjectesIdentify> dependency;

	SL_CLASS_MAPPING_BEGIN(COMObjectesIdentify)
		SL_MEMBER_V(clsid);
		SL_MEMBER_V(name);
		SL_MEMBER_V(desc);
		SL_MEMBER_V(dllname);
		SL_MEMBER_V(dllpath);
		SL_MEMBER_V(dependency);
	SL_CLASS_MAPPING_END()
};
inline COMObjectesIdentify::COMObjectesIdentify(){}
inline COMObjectesIdentify::~COMObjectesIdentify(){}

class CPrgCOMScaner
{
public:
    static HRESULT Scan(const base::FilePath &dir, std::list<COMObjectesIdentify> &identifies, PlatformDisableCOMObjectes &platformDiable);
	static HRESULT ScanUser(const base::FilePath &dir, UserDisableCOMObjectes &userDisable);
	static HRESULT SaveUserDefine(const base::FilePath &dir, UserDisableCOMObjectes &userDisable);
};

#endif // #ifndef __PrgCOMScaner_H_C9362A6A_BBF8_4683_8517_FCC330077845_INCLUDED__
