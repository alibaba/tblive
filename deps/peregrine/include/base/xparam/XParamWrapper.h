//------------------------------------------------------------------------------
//
//    版权所有(C) 浙江天猫技术有限公司 保留所有权利
//
//    创建者:   哈尔
//    创建日期: 2015-2-4
//    功能描述: 提供万能参数的封装实现，CXParamWrapper类实现了IXParam接口
//
//------------------------------------------------------------------------------

#ifndef __XPARAMWRAPPER_H_A0DF73FD_6FE8_44c9_A6D6_70D0C0FC078D_INCLUDED__
#define __XPARAMWRAPPER_H_A0DF73FD_6FE8_44c9_A6D6_70D0C0FC078D_INCLUDED__

#include <base/namespace.h>
#include <base/memory/ref_ptrcounted.h>
#include <base/xparam/IXParam.h>
#include <base/xparam/XParamCore.h>

#include "base/synchronization/lock.h"

class CXParamWrapper : public base::RefPtrCounted<prg::IXParam, base::ThreadSafeRefPolicy>
{
public:
    CXParamWrapper(void);
    virtual ~CXParamWrapper(void);

    inline xparam::XParamP GetProperXParam(const std::wstring& strName)
    {
        if (strName.length() > 0)
            return m_pCppXparam->GetFirstChild(strName.c_str(), strName.length());
        else
            return m_pCppXparam;
    }
    inline xparam::XParamP PutProperXParam(const std::wstring& strName)
    {
        if (strName.length() > 0)
        {
            unsigned int nlen = (unsigned int)strName.length();
            xparam::XParamP p = m_pCppXparam->GetFirstChild(strName.c_str(), nlen);
            if (!p)
            {
                p = m_pCppXparam->CreateXParam();
                p->SetName(strName.c_str(), nlen);
                m_pCppXparam->PushBackChild(p);
            }
            return p;
        }
        else
            return m_pCppXparam;
    }

    template <typename T> 
    inline HRESULT GetSimpleType(const std::wstring& strName, T *pVal)
    {
        if (!pVal)
            return E_POINTER;
        xparam::XParamP p = GetProperXParam(strName);
        if (!p) return E_FAIL;
        return p->GetValue(*pVal) ? S_OK : DISP_E_TYPEMISMATCH;
    }
    template <typename T> 
    inline HRESULT PutSimpleType(const std::wstring& strName, T newVal)
    {
        xparam::XParamP p = PutProperXParam(strName);
        p->SetValue(newVal);
        return S_OK;
    }
    inline void SetCppXParamImpl(xparam::XParamP const &src)
    {
        m_pCppXparam = src;
    }

    xparam::XParamP m_pCppXparam;

public:
    virtual HRESULT get_Char(const std::wstring& strName, char* pVal) OVERRIDE;
    virtual HRESULT put_Char(const std::wstring& strName, char newVal) OVERRIDE;
    virtual HRESULT get_UChar(const std::wstring& strName, unsigned char* pVal) OVERRIDE;
    virtual HRESULT put_UChar(const std::wstring& strName, unsigned char newVal) OVERRIDE;
    virtual HRESULT get_Short(const std::wstring& strName, short* pVal) OVERRIDE;
    virtual HRESULT put_Short(const std::wstring& strName, short newVal) OVERRIDE;
    virtual HRESULT get_UShort(const std::wstring& strName, unsigned short* pVal) OVERRIDE;
    virtual HRESULT put_UShort(const std::wstring& strName, unsigned short newVal) OVERRIDE;
    virtual HRESULT get_Long(const std::wstring& strName, long* pVal) OVERRIDE;
    virtual HRESULT put_Long(const std::wstring& strName, long newVal) OVERRIDE;
    virtual HRESULT get_ULong(const std::wstring& strName, unsigned long* pVal) OVERRIDE;
    virtual HRESULT put_ULong(const std::wstring& strName, unsigned long newVal) OVERRIDE;
    virtual HRESULT get_LongLong(const std::wstring& strName, long long* pVal) OVERRIDE;
    virtual HRESULT put_LongLong(const std::wstring& strName, long long newVal) OVERRIDE;
    virtual HRESULT get_ULongLong(const std::wstring& strName, unsigned long long* pVal) OVERRIDE;
    virtual HRESULT put_ULongLong(const std::wstring& strName, unsigned long long newVal) OVERRIDE;
    virtual HRESULT get_Int(const std::wstring& strName, int* pVal) OVERRIDE;
    virtual HRESULT put_Int(const std::wstring& strName, int newVal) OVERRIDE;
    virtual HRESULT get_UInt(const std::wstring& strName, unsigned int* pVal) OVERRIDE;
    virtual HRESULT put_UInt(const std::wstring& strName, unsigned int newVal) OVERRIDE;
    virtual HRESULT get_Float(const std::wstring& strName, float* pVal) OVERRIDE;
    virtual HRESULT put_Float(const std::wstring& strName, float newVal) OVERRIDE;
    virtual HRESULT get_Double(const std::wstring& strName, double* pVal) OVERRIDE;
    virtual HRESULT put_Double(const std::wstring& strName, double newVal) OVERRIDE;
    virtual HRESULT get_Bool(const std::wstring& strName, bool* pVal) OVERRIDE;
    virtual HRESULT put_Bool(const std::wstring& strName, bool newVal) OVERRIDE;
    virtual HRESULT get_Pvoid(const std::wstring& strName, void** pVal) OVERRIDE;
    virtual HRESULT put_PVoid(const std::wstring& strName, void* newVal) OVERRIDE;
    virtual HRESULT get_Str(const std::wstring& strName, std::wstring& val) OVERRIDE;
    virtual HRESULT put_Str(const std::wstring& strName, const std::wstring&newVal) OVERRIDE;
    virtual HRESULT get_MultiStr(const std::wstring& strName, std::string& val) OVERRIDE;
    virtual HRESULT put_MultiStr(const std::wstring& strName, const std::string& newVal) OVERRIDE;
	virtual HRESULT get_RefPtr(const std::wstring& strName, base::IRefCounted **pVal) OVERRIDE;
	virtual HRESULT put_RefPtr(const std::wstring& strName, base::IRefCounted *newVal) OVERRIDE;

#if defined(OS_WIN)
    virtual HRESULT get_Unknown(const std::wstring& strName, IUnknown** pVal) OVERRIDE;
    virtual HRESULT put_Unknown(const std::wstring& strName, IUnknown* newVal) OVERRIDE;
    virtual HRESULT get_Dispatch(const std::wstring& strName, IDispatch** pVal) OVERRIDE;
    virtual HRESULT put_Dispatch(const std::wstring& strName, IDispatch* newVal) OVERRIDE;
    virtual HRESULT get_Bstr(const std::wstring& strName, BSTR* pVal) OVERRIDE;
    virtual HRESULT put_Bstr(const std::wstring& strName, BSTR newVal) OVERRIDE;
    virtual HRESULT get_Value(const std::wstring& strName, VARIANT* pVal) OVERRIDE;
    virtual HRESULT put_Value(const std::wstring& strName, const VARIANT& newVal) OVERRIDE;
    virtual HRESULT CreateEnum(prg::IXParamEnum** ppEnum) OVERRIDE;
#endif

    virtual HRESULT Clone(bool bCopy, IXParam** ppXParam) OVERRIDE;
    virtual HRESULT Clear() OVERRIDE;
    virtual HRESULT Remove(const std::wstring& strName) OVERRIDE;
    virtual HRESULT Exchange(prg::IXParam* pXParam) OVERRIDE;

    virtual HRESULT GetCppXParam(void** ppCppXParam) OVERRIDE;
    virtual HRESULT SetCppXParam(void* pCppXParam) OVERRIDE;
    virtual HRESULT GetParent(prg::IXParam** ppXParam) OVERRIDE;
    virtual HRESULT PushBackChild(prg::IXParam* pChild) OVERRIDE;
    virtual HRESULT PushFrontChild(prg::IXParam* pChild) OVERRIDE;
    virtual HRESULT InsertBefore(prg::IXParam* pBefore, prg::IXParam* pChild) OVERRIDE;
    virtual HRESULT InsertAfter(prg::IXParam* pAfter, prg::IXParam* pChild) OVERRIDE;
    virtual HRESULT DropChild(prg::IXParam* pChild) OVERRIDE;
    virtual HRESULT DropSelf() OVERRIDE;
    virtual HRESULT ClearChildren() OVERRIDE;
    virtual HRESULT GetFirstChild(prg::IXParam** ppXParam) OVERRIDE;
    virtual HRESULT GetFirstChildByName(const std::wstring& strName, unsigned long nOffset, prg::IXParam** ppXParam) OVERRIDE;
    virtual HRESULT GetLastChild(prg::IXParam** ppXParam) OVERRIDE;
    virtual HRESULT GetLastChildByName(const std::wstring& strName, unsigned long nOffset, prg::IXParam** ppXParam) OVERRIDE;
    virtual HRESULT GetChildrenCount(unsigned long* pCount) OVERRIDE;
    virtual HRESULT GetPrevSibling(prg::IXParam** ppXParam) OVERRIDE;
    virtual HRESULT GetPrevNamesake(prg::IXParam** ppXParam) OVERRIDE;
    virtual HRESULT GetNextSibling(prg::IXParam** ppXParam) OVERRIDE;
    virtual HRESULT GetNextNamesake(prg::IXParam** ppXParam) OVERRIDE;
    virtual HRESULT GetName(std::wstring& strName) OVERRIDE;
    virtual HRESULT SetName(const std::wstring& strName) OVERRIDE;
    virtual HRESULT LockIt() OVERRIDE;
    virtual HRESULT UnlockIt() OVERRIDE;
    virtual HRESULT FromXmlString(const std::wstring& strXmlString) OVERRIDE;
    virtual HRESULT ToXmlString(const std::wstring& strIndentString, const std::wstring& strCrlfString, std::wstring& strXmlString) OVERRIDE;
    virtual HRESULT FromXmlMemory(void* pXmlMemory, unsigned int XmlSize) OVERRIDE;
    virtual HRESULT FromXmlFile(const base::FilePath &filePath) OVERRIDE;
    virtual HRESULT ToXmlFile(const base::FilePath &filePath, const std::wstring& strIndentString, const std::wstring& strCrlfString, const std::wstring& strEncoding) OVERRIDE;

private:
    base::Lock m_lock;
};

#endif // #ifndef __XPARAMWRAPPER_H_A0DF73FD_6FE8_44c9_A6D6_70D0C0FC078D_INCLUDED__
