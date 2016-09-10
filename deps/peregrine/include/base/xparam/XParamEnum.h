//------------------------------------------------------------------------------
//
//    版权所有(C) 浙江天猫技术有限公司 保留所有权利
//
//    创建者:   哈尔
//    创建日期: 2015-2-5
//    功能描述: 提供万能参数遍历接口实现
//
//------------------------------------------------------------------------------

#ifndef __XPARAMENUM_H_B2CD349C_7C10_482B_A2B1_310074614C8D_INCLUDED__
#define __XPARAMENUM_H_B2CD349C_7C10_482B_A2B1_310074614C8D_INCLUDED__

#include <base/namespace.h>
#include <base/memory/ref_counted.h>
#include <base/memory/ref_ptrcounted.h>
#include <base/xparam/IXParam.h>

class CXParamEnum: public base::RefPtrCounted<prg::IXParamEnum, base::ThreadSafeRefPolicy>
{
public:
    CXParamEnum(void);
    virtual ~CXParamEnum(void) {Finalize();}

    void Initialize(prg::IXParam* p)
    {
        m_p = p;
        if (p)
            p->LockIt();
    }

    void Finalize()
    {
        if (m_p)
            m_p->UnlockIt();
    }

    virtual HRESULT GetName(std::wstring& strName);
    virtual HRESULT GetValue(VARIANT* pVal);
    virtual HRESULT Next(bool* pEOF);
    virtual HRESULT Reset();

private:
    scoped_refptr<prg::IXParam> m_p;
    scoped_refptr<prg::IXParam> m_ch;
};

#endif // #ifndef __XPARAMENUM_H_B2CD349C_7C10_482B_A2B1_310074614C8D_INCLUDED__