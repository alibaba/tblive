//------------------------------------------------------------------------------
//
//    版权所有(C) 浙江天猫技术有限公司 保留所有权利
//
//    创建者:   哈尔
//    创建日期: 2015-2-4
//    功能描述: XParam的接口定义
//
//------------------------------------------------------------------------------

#ifndef __IXPARAM_H_DF8F9CEA_1714_420A_A763_4483B444E4F3_INCLUDED__
#define __IXPARAM_H_DF8F9CEA_1714_420A_A763_4483B444E4F3_INCLUDED__

#include <base/hresult_define.h>
#include <base/memory/iref_counted.h>
#include <base/namespace.h>
#include <base/files/file_path.h>
#include <string>

#if defined(OS_WIN)
#include <windows.h>
#include <unknwn.h>
#include <oaidl.h>
#endif

BEGIN_NAMESPACE(prg)

//typedef struct tagGUIXBLOB
//{
//    ULONG cbSize;
//    [size_is(cbSize)] BYTE* pData;
//}GUIXBLOB;

// 遍历功能目前仅提供给RVGUI库使用
#if defined(OS_WIN)
class IXParamEnum : public base::IRefCounted
{
public:
    virtual HRESULT GetName(std::wstring& strName) = 0;
    virtual HRESULT GetValue(VARIANT* pVal) = 0;
    virtual HRESULT Next(bool* pEOF) = 0;
    virtual HRESULT Reset() = 0;
};
#endif

class IXParam : public base::IRefCounted
{
public:
    virtual HRESULT get_Char(const std::wstring& strName, char* pVal) = 0;
    virtual HRESULT put_Char(const std::wstring& strName, char newVal) = 0;
    virtual HRESULT get_UChar(const std::wstring& strName, unsigned char* pVal) = 0;
    virtual HRESULT put_UChar(const std::wstring& strName, unsigned char newVal) = 0;
    virtual HRESULT get_Short(const std::wstring& strName, short* pVal) = 0;
    virtual HRESULT put_Short(const std::wstring& strName, short newVal) = 0;
    virtual HRESULT get_UShort(const std::wstring& strName, unsigned short* pVal) = 0;
    virtual HRESULT put_UShort(const std::wstring& strName, unsigned short newVal) = 0;
    virtual HRESULT get_Long(const std::wstring& strName, long* pVal) = 0;
    virtual HRESULT put_Long(const std::wstring& strName, long newVal) = 0;
    virtual HRESULT get_ULong(const std::wstring& strName, unsigned long* pVal) = 0;
    virtual HRESULT put_ULong(const std::wstring& strName, unsigned long newVal) = 0;
    virtual HRESULT get_LongLong(const std::wstring& strName, long long* pVal) = 0;
    virtual HRESULT put_LongLong(const std::wstring& strName, long long newVal) = 0;
    virtual HRESULT get_ULongLong(const std::wstring& strName, unsigned long long* pVal) = 0;
    virtual HRESULT put_ULongLong(const std::wstring& strName, unsigned long long newVal) = 0;
    virtual HRESULT get_Int(const std::wstring& strName, int* pVal) = 0;
    virtual HRESULT put_Int(const std::wstring& strName, int newVal) = 0;
    virtual HRESULT get_UInt(const std::wstring& strName, unsigned int* pVal) = 0;
    virtual HRESULT put_UInt(const std::wstring& strName, unsigned int newVal) = 0;
    virtual HRESULT get_Float(const std::wstring& strName, float* pVal) = 0;
    virtual HRESULT put_Float(const std::wstring& strName, float newVal) = 0;
    virtual HRESULT get_Double(const std::wstring& strName, double* pVal) = 0;
    virtual HRESULT put_Double(const std::wstring& strName, double newVal) = 0;
    virtual HRESULT get_Bool(const std::wstring& strName, bool* pVal) = 0;
    virtual HRESULT put_Bool(const std::wstring& strName, bool newVal) = 0;
    virtual HRESULT get_Pvoid(const std::wstring& strName, void** pVal) = 0;
    virtual HRESULT put_PVoid(const std::wstring& strName, void* newVal) = 0;
    virtual HRESULT get_Str(const std::wstring& strName, std::wstring& val) = 0;
    virtual HRESULT put_Str(const std::wstring& strName, const std::wstring& newVal) = 0;
    virtual HRESULT get_MultiStr(const std::wstring& strName, std::string& val) = 0;
    virtual HRESULT put_MultiStr(const std::wstring& strName, const std::string& newVal) = 0;
	virtual HRESULT get_RefPtr(const std::wstring& strName, base::IRefCounted **pVal) = 0;
	virtual HRESULT put_RefPtr(const std::wstring& strName, base::IRefCounted *newVal) = 0;

#if defined(OS_WIN)
    virtual HRESULT get_Unknown(const std::wstring& strName, IUnknown** pVal) = 0;
    virtual HRESULT put_Unknown(const std::wstring& strName, IUnknown* newVal) = 0;
    virtual HRESULT get_Dispatch(const std::wstring& strName, IDispatch** pVal) = 0;
    virtual HRESULT put_Dispatch(const std::wstring& strName, IDispatch* newVal) = 0;
    virtual HRESULT get_Bstr(const std::wstring& strName, BSTR* pVal) = 0;
    virtual HRESULT put_Bstr(const std::wstring& strName, BSTR newVal) = 0;
    virtual HRESULT get_Value(const std::wstring& strName, VARIANT* pVal) = 0;
    virtual HRESULT put_Value(const std::wstring& strName, const VARIANT& newVal) = 0;
    virtual HRESULT CreateEnum(IXParamEnum** ppEnum) = 0;
#endif

    virtual HRESULT Clone(bool bCopy, IXParam** ppXParam) = 0;
    virtual HRESULT Clear() = 0;
    virtual HRESULT Remove(const std::wstring& strName) = 0;
    virtual HRESULT Exchange(IXParam* pXParam) = 0;

    virtual HRESULT GetCppXParam(void** ppCppXParam) = 0;
    virtual HRESULT SetCppXParam(void* pCppXParam) = 0;
    virtual HRESULT GetParent(IXParam** ppXParam) = 0;
    virtual HRESULT PushBackChild(IXParam* pChild) = 0;
    virtual HRESULT PushFrontChild(IXParam* pChild) = 0;
    virtual HRESULT InsertBefore(IXParam* pBefore, IXParam* pChild) = 0;
    virtual HRESULT InsertAfter(IXParam* pAfter, IXParam* pChild) = 0;
    virtual HRESULT DropChild(IXParam* pChild) = 0;
    virtual HRESULT DropSelf() = 0;
    virtual HRESULT ClearChildren() = 0;
    virtual HRESULT GetFirstChild(IXParam** ppXParam) = 0;
    virtual HRESULT GetFirstChildByName(const std::wstring& strName, unsigned long nOffset, IXParam** ppXParam) = 0;
    virtual HRESULT GetLastChild(IXParam** ppXParam) = 0;
    virtual HRESULT GetLastChildByName(const std::wstring& strName, unsigned long nOffset, IXParam** ppXParam) = 0;
    virtual HRESULT GetChildrenCount(unsigned long* pCount) = 0;
    virtual HRESULT GetPrevSibling(IXParam** ppXParam) = 0;
    virtual HRESULT GetPrevNamesake(IXParam** ppXParam) = 0;
    virtual HRESULT GetNextSibling(IXParam** ppXParam) = 0;
    virtual HRESULT GetNextNamesake(IXParam** ppXParam) = 0;
    virtual HRESULT GetName(std::wstring& strName) = 0;
    virtual HRESULT SetName(const std::wstring& strName) = 0;
    virtual HRESULT LockIt() = 0;
    virtual HRESULT UnlockIt() = 0;
    virtual HRESULT FromXmlString(const std::wstring& strXmlString) = 0;
    virtual HRESULT ToXmlString(const std::wstring& strIndentString, const std::wstring& strCrlfString, std::wstring& strXmlString) = 0;
    virtual HRESULT FromXmlMemory(void* pXmlMemory, unsigned int XmlSize) = 0;
    virtual HRESULT FromXmlFile(const base::FilePath &filePath) = 0;
    virtual HRESULT ToXmlFile(const base::FilePath &filePath, const std::wstring& strIndentString, const std::wstring& strCrlfString, const std::wstring& strEncoding) = 0;
};

END_NAMESPACE()

#endif // #ifndef __IXPARAM_H_DF8F9CEA_1714_420A_A763_4483B444E4F3_INCLUDED__
