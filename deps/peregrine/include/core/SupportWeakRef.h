//------------------------------------------------------------------------------
//
//  版权所有(C) 浙江天猫技术有限公司 保留所有权利
//
//  创建者:   哈尔
//  创建日期: 2015/02/16
//  功能描述: 支持弱引用的实现
//
//------------------------------------------------------------------------------

#ifndef __SUPPORTWEAKREF_H_FBBB866D_0C4C_46C9_9A78_520245B01625_INCLUDED__
#define __SUPPORTWEAKREF_H_FBBB866D_0C4C_46C9_9A78_520245B01625_INCLUDED__

#include <base/namespace.h>
#include <base/memory/ref_counted.h>
#include <core/ISupportWeakRef.h>
#include <core/PrgCOMRefCounted.h>
#include <core/PrgCOMBase.h>
#include <core/PrgCore.h>

BEGIN_NAMESPACE(prg)

template <typename Interface>
class CSupportWeakRef : public prg::CPrgCOMRootObject<prg::CCOMThreadSafeWeakRefPolicy>, public Interface
{
public:
    CSupportWeakRef()
    {
        HRESULT hr = CreateWeakRef(m_spWeakRef.getaddress());
        if (SUCCEEDED(hr))
        {
            m_spWeakRef->BindStrongRef(this, &weak_ref_count_);
        }
    }
    HRESULT GetWeakRef(prg::IWeakRef **ppWeakRef)
    {
        if (ppWeakRef == NULL)
        {
            return E_INVALIDARG;
        }
        
        m_spWeakRef->AddRef();
        *ppWeakRef = m_spWeakRef;
        return S_OK;
    }
    HRESULT GetStrongRef(const std::string &iid, void **ppObject)
    {
        return this->QueryInterface(iid, ppObject);
    }

private:
    scoped_refptr<prg::IWeakRef> m_spWeakRef;
};

END_NAMESPACE()

#endif // __SUPPORTWEAKREF_H_FBBB866D_0C4C_46C9_9A78_520245B01625_INCLUDED__

