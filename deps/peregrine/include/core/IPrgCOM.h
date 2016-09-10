//------------------------------------------------------------------------------
//
//    版权所有(C) 阿里巴巴（中国）网络技术有限公司 保留所有权利
//
//    创建者:   徐明
//    创建日期: 2015-01-29
//    功能描述: 提供跨平台二进制文件的对象创建方案，类似Windows下COM的解决方案。
//
//
//------------------------------------------------------------------------------

/**
* @class IPrgCOM
* @brief
* @author 徐明
* @version 1.0.0
*/

#ifndef __IPrgCOM_H_75895926_20F7_40B4_A1F4_718D2F4AFFA1_INCLUDED__
#define __IPrgCOM_H_75895926_20F7_40B4_A1F4_718D2F4AFFA1_INCLUDED__

#include "string"
#include "base/namespace.h"
#include "base/prg.h"
#include "base/memory/iref_counted.h"
#include "base/memory/ref_counted.h"
#include "list"
#include "base/files/file_path.h"

BEGIN_NAMESPACE(prg)

class CPrgCOMObjectIdentify:public base::RefCountedThreadSafe<CPrgCOMObjectIdentify>
{
public:
    CPrgCOMObjectIdentify();

	std::string clsid;
	std::string name;
	std::string desc;
	std::string dllname;
	bool enable;
	std::list<scoped_refptr<CPrgCOMObjectIdentify>> dependency;
    
private:
    friend class base::RefCountedThreadSafe<CPrgCOMObjectIdentify>;
    ~CPrgCOMObjectIdentify();
};
inline CPrgCOMObjectIdentify::CPrgCOMObjectIdentify():enable(true){}
inline CPrgCOMObjectIdentify::~CPrgCOMObjectIdentify(){}

class IPrgCOM:public base::IRefCounted
{
public:

     /**
     * @brief 初始化COM库
     * @param path 数据路径，PrgCOM会扫描底下的所有dll(Windows下的dll,MacOS下的dylib)，获取模块依赖关系生成PrgCOMObjectIdentifies.xml文件，如果有此xml文件，不重复扫描。
	 * PrgCOM会读取PrgDisableCOM.xml，初始化当前应用级别不需要加载的模块列表。
     */
    virtual HRESULT Init(const base::FilePath &path) = 0;
	
	 /**
     * @brief 初始化自定义（例如每个登陆账号相关）的PrgDisableCOM.xml,由于一个进程内可以定义多份自定义，所以需要引入configid
     * @param userPath 数据路径，PrgCOM会读取PrgDisableCOM，生成自定义的不能加载的模块列表。
     */
	virtual HRESULT InitCustomPrgCOMObjectConfig(const base::FilePath &userPath, const std::string &configid = "Id1") = 0;

    /**
    * @brief 反初始化COM库
    */
    virtual HRESULT Uninit() = 0;

    /**
    * @brief 创造一个新的COM实例
    * @param clsid COM实例classid 
    * @param iid 实例欲获取的接口 
    * @param ppObject 实例接口 
	* @param configid 自定义的COM加载策略id
    * @return hresult_define.h
    * @remark 
    */
    virtual HRESULT CreateInstance(const std::string &clsid, const std::string &iid, void **ppObject, const std::string &configid = "") = 0;
    
    /**
    * @brief 获取COM实例，如果曾经Get过，返回Cache，不然创建新的实例
    * @param clsid COM实例classid 
    * @param name 欲创建的实例命名
    * @param iid 实例欲获取的接口 
    * @param ppObject 实例接口
	* @param configid 自定义的COM加载策略id
	* @param groupId 对象分组id，用于缓存的对象隔离
    * @return hresult_define.h
    * @remark 
    */
    virtual HRESULT GetInstance(const std::string &clsid, const std::string &name, const std::string &iid, void **ppObject, const std::string &configid = "", const std::string &groupId = "") = 0;
    
    /**
    * @brief 判断Cache库中是否有此COM实例
    * @param clsid COM实例classid 
    * @param name 欲询问的实例命名
    * @param bHas 是否存在 
	* @param groupId 对象分组id，用于缓存的对象隔离
    * @return S_OK
    * @remark 
    */
    virtual HRESULT HasInstance(const std::string &clsid, const std::string &name, bool *bHas, const std::string &groupId = "") = 0;
    
    /**
    * @brief 删除Cache库中的实例
    * @param clsid COM实例classid 
    * @param name 欲删除的实例命名
	* @param groupId 对象分组id，用于缓存的对象隔离
    * @return S_OK
    * @remark 
    */
    virtual HRESULT DropInstance(const std::string &clsid, const std::string &name, const std::string &groupId = "") = 0;

	/**
    * @brief 删除Cache库中此分组下的所有实例
	* @param groupId 对象分组id，用于缓存的对象隔离
    * @return S_OK
    * @remark 
    */
	virtual HRESULT DropInstanceGroup(const std::string &groupId = "") = 0;
	/**
    * @brief 是否允许创建实例
    * @return S_OK
    * @remark 
    */
	virtual HRESULT CanCreateInstance(const std::string &clsid, bool *bCan, const std::string &configid = "") = 0;

	/**
    * @brief 获取当前PrgCOM的依赖关系树
    * @return S_OK
    * @remark 
    */
	virtual HRESULT GetPrgCOMObjectConfig(std::list<scoped_refptr<CPrgCOMObjectIdentify>> &listPrgCOMObject, const std::string &configid = "") = 0;

	/**
    * @brief 重置用户自定义的disable加载模块
	* @param configid 自定义的COM加载策略id
    * @return S_OK
    * @remark 
    */
	virtual HRESULT ResetCustomPrgCOMObjectConfig(const std::string &configid = "Id1") = 0;

	/**
    * @brief 设置用户自定义的disable加载模块
	* @param configid 自定义的COM加载策略id
    * @return S_OK
    * @remark 
    */
	virtual HRESULT EnableCustomPrgCOMObjectConfig(const std::string &clsid, bool enable = true, const std::string &configid = "Id1") = 0;
};

END_NAMESPACE()
#endif // #ifndef __IPrgCOM_H_75895926_20F7_40B4_A1F4_718D2F4AFFA1_INCLUDED__
