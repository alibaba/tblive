//------------------------------------------------------------------------------
//
//    版权所有(C) 阿里巴巴（中国）网络技术有限公司 保留所有权利
//
//    创建者:   徐明
//    创建日期: 2011-11-07
//    功能描述: 消息中心容器
//
//
//------------------------------------------------------------------------------

#ifndef __PRGMCCONTAINER_H_6F707BB7_9541_46CC_8676_BC12FC1936B2_INCLUDED__
#define __PRGMCCONTAINER_H_6F707BB7_9541_46CC_8676_BC12FC1936B2_INCLUDED__

#include <core/IPrgMC.h>
#include <base/memory/ref_counted.h>
#include "base/synchronization/lock.h"
#include <map>

class CPrgMCContainer:public base::RefCounted<CPrgMCContainer>
{
public:
    CPrgMCContainer();

    HRESULT GetPrgMessageCenter(const std::wstring& strName, prg::IPrgMC **ppPrgMC);
    HRESULT DestroyRrgMessageCenter(const std::wstring& strName);
    
private:
    friend class base::RefCounted<CPrgMCContainer>;
    ~CPrgMCContainer();

private:
    std::map<std::wstring, scoped_refptr<prg::IPrgMC>> m_MCs;
    base::Lock m_lock;
};



#endif // #ifndef __PRGMCCONTAINER_H_6F707BB7_9541_46CC_8676_BC12FC1936B2_INCLUDED__
