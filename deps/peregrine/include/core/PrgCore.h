//------------------------------------------------------------------------------
//
//    版权所有(C) 阿里巴巴（中国）网络技术有限公司 保留所有权利
//
//    创建者:   徐明
//    创建日期: 2015-1-29
//    功能描述: PrgCore的导出函数
//
//
//------------------------------------------------------------------------------

#ifndef __PrgCore_H_6063515E_090A_446C_B30C_039EE75D0E49_INCLUDED__
#define __PrgCore_H_6063515E_090A_446C_B30C_039EE75D0E49_INCLUDED__

#include "base/base_export.h"
#include "core/IPrgCOM.h"
#include "core/IPrgMC.h"
#include "core/IWeakRef.h"

BASE_EXPORT HRESULT GetPrgCOM(prg::IPrgCOM **ppPrgCOM);

/**
* @brief 获取消息中心的实例接口
* @param strName [in] 消息中心的命名
* @param ppPrgMC [out] 消息中心的实例接口
* @return 成功返回S_OK，失败返回具体的错误ID
* @remark 通过strName获取不同的实例接口
*/
BASE_EXPORT HRESULT GetPrgMessageCenter(const std::wstring& strName, prg::IPrgMC **ppPrgMC);

/**
* @brief 销毁消息中心的实例接口
* @param strName [in] 消息中心的命名
* @return 成功返回S_OK，失败返回具体的错误ID
* @remark 请确保销毁前，调用过UnInit
*/
BASE_EXPORT HRESULT DestroyPrgMessageCenter(const std::wstring& strName);

/**
* @brief 创建弱引用对象
* @param IWeakRef [in] 弱引用对象二级指针
* @return 成功返回S_OK，失败返回具体的错误ID
* @remark
*/
BASE_EXPORT HRESULT CreateWeakRef(prg::IWeakRef** ppWeakRef);


#endif // #ifndef __PrgCore_H_6063515E_090A_446C_B30C_039EE75D0E49_INCLUDED__
