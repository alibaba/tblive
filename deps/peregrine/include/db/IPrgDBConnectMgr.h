//------------------------------------------------------------------------------
//
//    版权所有(C) 阿里巴巴（中国）网络技术有限公司 保留所有权利
//
//    创建者:   徐明
//    创建日期: 2015-2-28
//    功能描述: 针对于sqlite3的多线程模型，封装每一个线程一个sqlite*的逻辑
//
//
//------------------------------------------------------------------------------
#pragma once

#include "base/namespace.h"
#include "base/prg.h"
#include "base/memory/iref_counted.h"
#include "string"
#include "base/files/file_path.h"
#include "db/IPrgDBConnection.h"

BEGIN_NAMESPACE(prg)

class IPrgDBConnectMgr:public base::IRefCounted
{
public:
	/**
	* @brief 初始化DB信息,以便后续获取IPrgDBConnect等
	* @param name [in] db文件对应的id，可以理解为db文件的别名，当然也可以定义为表名。
	* @param dbPath [in] db文件路径
	* @param dbKey [in] db文件加密秘钥
	* @return 成功返回S_OK，失败返回具体的错误ID
	*/
	virtual HRESULT InitConnection(const std::string &name, const base::FilePath &dbPath, const std::string &dbKey, const bool superDB = false) = 0;

    /**
     * @brief 初始化DB信息,以便后续获取IPrgDBConnect等
     * @param name [in] db文件对应的id，可以理解为db文件的别名，当然也可以定义为表名。
     * @param dbPath [in] db文件路径
     * @param dbKey [in] db文件加密秘钥
     * @return 成功返回S_OK，失败返回具体的错误ID
     */
    virtual HRESULT UninitConnection(const std::string &name) = 0;

	/**
	* @brief 获取符合当前线程的IPrgDBConnection接口，同时PrgDBConnectMgr会存储此IPrgDBConnection直到调用CloseConnection等接口
	* @param name [in] db文件对应的id
	* @param IPrgDBConnection [out] IPrgDBConnection接口
	* @return 成功返回S_OK，失败返回具体的错误ID
	*/
	virtual HRESULT GetConnection(const std::string &name, prg::IPrgDBConnection **ppPrgDBConnection) = 0;

	/**
	* @brief 关闭PrgDBConnectMgr存储的当前线程的PrgDBConnect对象
	* @param name [in] 具体关闭的db文件id，如果为空清除所有。
	* @return 成功返回S_OK，失败返回具体的错误ID
	*/
	virtual HRESULT CloseCurrentThreadConnection(const std::string &name) = 0;

	/**
	* @brief 关闭PrgDBConnectMgr存储的PrgDBConnect对象
	* @param name [in] 具体关闭的db文件id，如果为空清除所有。
	* @return 成功返回S_OK，失败返回具体的错误ID
	*/
	virtual HRESULT CloseConnection(const std::string &name) = 0;

	/**
	* @brief 创建ppPrgDBConnect对象，不同于GetConnection，PrgDBConnectMgr不存储IPrgDBConnection
	* @param IPrgDBConnection [out] IPrgDBConnection接口
	* @return 成功返回S_OK，失败返回具体的错误ID
	*/
	virtual HRESULT CreateConnection(const std::string &name, prg::IPrgDBConnection **ppPrgDBConnection) = 0;
	
	//======================================================
	//底下为旺旺变态需求引发的变态接口，别的应用可以不鸟。
	//为了保证数据库的加密秘钥是由服务器下发的，而下发的秘钥又需要在本地有内容去映射换取，如果以另外的文件存储会造成该文件和db的原子性，
	//所以最终这个映射数据放在了sqlite文件里面。
	
	/**
	* @brief 判断是否是SuperDB
	* @param dbPath [in] db文件路径
	* @param pSuperDB [out]
	* @return 成功返回S_OK，失败返回具体的错误ID
	*/
	virtual HRESULT IsSuperDB(const base::FilePath &dbPath, bool *pSuperDB) = 0;

	/**
	* @brief 获取SuperDB的映射数据
	* @param dbPath [in] db文件路径
	* @param nLen [in] 获取的数据长度
	* @param pBuf [out]
	* @return 成功返回S_OK，失败返回具体的错误ID
	*/
	virtual HRESULT GetSuperDBHead(const base::FilePath &dbPath, const long nLen, unsigned char *pBuf) = 0;

	/**
	* @brief 更新SuperDB的映射数据
	* @param dbPath [in] db文件路径
	* @param pBuf [in]
	* @param nLen [in] 获取的数据长度	
	* @return 成功返回S_OK，失败返回具体的错误ID
	*/
	virtual HRESULT UpdateSuperHead(const base::FilePath &dbPath, const unsigned char *pBuf, long nLen) = 0;
	//======================================================
};

END_NAMESPACE()
