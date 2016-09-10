#ifndef __XParamHelper_H_15C73818_8FD8_4B5F_A3BA_9971D11CE2EC_INCLUDED__
#define __XParamHelper_H_15C73818_8FD8_4B5F_A3BA_9971D11CE2EC_INCLUDED__

#include <base/xparam/IXParam.h>
#include <base/xparam/XParam.h>
#include <base/namespace.h>
#include <base/memory/ref_counted.h>
#include <base/memory/iref_counted_wrap.h>
#include <base/srlz/archive_xparam.h>

#if defined(OS_WIN)
#include <comutil.h>
#include <comdef.h>
#include <comdefsp.h>
#include <atlstr.h>
#endif

BEGIN_NAMESPACE(prg)

typedef scoped_refptr<prg::IXParam> IXParamPtr;
#if defined(OS_WIN)
typedef scoped_refptr<prg::IXParamEnum> IXParamEnumPtr;

class xpenum
{
public:
    std::wstring GetName() const
    {
        std::wstring strName;
        m_spEnum->GetName(strName);
        return strName;
    }
    _variant_t GetValue() const
    {
        _variant_t Value;
        m_spEnum->GetValue(Value.GetAddress());
        return Value;
    }
    bool Next() const
    {
        bool bEof;
        m_spEnum->Next(&bEof);
        return bEof;
    }
    void Reset() const
    {
        m_spEnum->Reset();
    }
    IXParamEnum* operator->() const
    {
        return m_spEnum;
    };
private:
    friend class prgxp;
    xpenum(IXParamEnum* pEnum)
        : m_spEnum(pEnum)
    {
        assert(pEnum != NULL);
    }
    IXParamEnumPtr m_spEnum;
};
#endif

class prgxp : public IRefcountWrapBase<IXParam, prgxp>
{
public:
	prgxp()
	{
	}
	prgxp(IXParam* pXParam)
		: wrapbase(pXParam)
	{
	}
	void CreateInPlace()
	{
        ::CreateXParam(m_spInterface.getaddress());
	}
	static prgxp CreateXParam()
	{
		IXParamPtr spXParam;
		::CreateXParam(spXParam.getaddress());
		return prgxp(spXParam);
	}

#define _REFERENCE_GET_PUT_METHODS(t, n) \
	bool Get##n(std::wstring const& Name, t& Val) const\
	{\
		return SUCCEEDED(m_spInterface->get_##n(Name, Val));\
	}\
	t _get_##n(std::wstring const& Name) const\
	{\
		t Val;\
		m_spInterface->get_##n(Name, Val);\
		return Val;\
	}\
	void _put_##n(std::wstring const& Name, t Val) const\
	{\
		m_spInterface->put_##n(Name, Val);\
	}
	
	//__declspec(property(get=_get_##n, put=_put_##n)) t n[];

    #define _GERNERIC_GET_PUT_METHODS(t, n) \
	bool Get##n(std::wstring const& Name, t& Val) const\
	{\
		return SUCCEEDED(m_spInterface->get_##n(Name, &Val));\
	}\
	t _get_##n(std::wstring const& Name) const\
	{\
		t Val;\
		m_spInterface->get_##n(Name, &Val);\
		return Val;\
	}\
	void _put_##n(std::wstring const& Name, t Val) const\
	{\
		m_spInterface->put_##n(Name, Val);\
	}
	
	//__declspec(property(get=_get_##n, put=_put_##n)) t n[];

#define _POINTER_GET_PUT_METHODS(t1, t2, t3, n, addr1) \
	bool Get##n(std::wstring const& Name, t1& Val) const\
	{\
		typedef t3 T3;\
		t3 newVal = T3();\
		bool bret = SUCCEEDED(m_spInterface->get_##n(Name, &newVal));\
		if (bret)\
			Val.Attach(newVal);\
		return bret;\
	}\
	t1 _get_##n(std::wstring const& Name) const\
	{\
		t1 Val;\
		m_spInterface->get_##n(Name, addr1);\
		return Val;\
	}\
	void _put_##n(std::wstring const& Name, t2 Val) const\
	{\
		m_spInterface->put_##n(Name, Val);\
	}
	
	//__declspec(property(get=_get_##n, put=_put_##n)) t1 n[];

	_GERNERIC_GET_PUT_METHODS(char, Char)
	_GERNERIC_GET_PUT_METHODS(unsigned char, UChar)
	_GERNERIC_GET_PUT_METHODS(short, Short)
	_GERNERIC_GET_PUT_METHODS(unsigned short, UShort)
	_GERNERIC_GET_PUT_METHODS(long, Long)
	_GERNERIC_GET_PUT_METHODS(unsigned long, ULong)
	_GERNERIC_GET_PUT_METHODS(long long, LongLong)
	_GERNERIC_GET_PUT_METHODS(unsigned long long, ULongLong)
	_GERNERIC_GET_PUT_METHODS(int, Int)
	_GERNERIC_GET_PUT_METHODS(unsigned int, UInt)
	_GERNERIC_GET_PUT_METHODS(float, Float)
	_GERNERIC_GET_PUT_METHODS(double, Double)
	_GERNERIC_GET_PUT_METHODS(bool, Bool)
    _REFERENCE_GET_PUT_METHODS(std::wstring, Str)
    _REFERENCE_GET_PUT_METHODS(std::string, MultiStr)
#if defined(OS_WIN)
	_POINTER_GET_PUT_METHODS(IUnknownPtr, IUnknown*, IUnknown*, Unknown, &Val)
	_POINTER_GET_PUT_METHODS(IDispatchPtr, IDispatch*, IDispatch*, Dispatch, &Val)
	_POINTER_GET_PUT_METHODS(_bstr_t, BSTR, BSTR, Bstr, Val.GetAddress())
	_POINTER_GET_PUT_METHODS(_variant_t, REFVARIANT, VARIANT, Value, Val.GetAddress())
#endif

#undef _POINTER_GET_PUT_METHODS
#undef _GERNERIC_GET_PUT_METHODS

	prgxp Clone(bool bCopy = true) const
	{
		IXParamPtr spNewXp;
		m_spInterface->Clone(bCopy, spNewXp.getaddress());
		return prgxp(spNewXp);
	}

	HRESULT Clear() const
	{
		return m_spInterface->Clear();
	}

	HRESULT Remove(const std::wstring& strName) const
	{
		return m_spInterface->Remove(strName);
	}

#if defined(OS_WIN)
	xpenum CreateEnum() const
	{
		IXParamEnumPtr spEnum;
		m_spInterface->CreateEnum(spEnum.getaddress());
		return xpenum(spEnum);
	}
#endif

	HRESULT Exchange(IXParam* pXParam) const
	{
		return m_spInterface->Exchange(pXParam);
	}

	prgxp GetParent() const
	{
		IXParamPtr spXParam;
		m_spInterface->GetParent(spXParam.getaddress());
		return prgxp(spXParam);
	}

	HRESULT PushBackChild(IXParam* pXParam) const
	{
		return m_spInterface->PushBackChild(pXParam);
	}

	HRESULT PushFrontChild(IXParam* pXParam) const
	{
		return m_spInterface->PushFrontChild(pXParam);
	}

	HRESULT InsertBefore(IXParam* pBefore, IXParam* pChild) const
	{
		return m_spInterface->InsertBefore(pBefore, pChild);
	}

	HRESULT InsertAfter(IXParam* pAfter, IXParam* pChild) const
	{
		return m_spInterface->InsertAfter(pAfter, pChild);
	}

	HRESULT DropChild(IXParam* pChild) const
	{
		return m_spInterface->DropChild(pChild);
	}

	HRESULT DropSelf() const
	{
		return m_spInterface->DropSelf();
	}

	HRESULT ClearChildren() const
	{
		return m_spInterface->ClearChildren();
	}

	prgxp GetFirstChild() const
	{
		IXParamPtr spXParam;
		m_spInterface->GetFirstChild(spXParam.getaddress());
		return prgxp(spXParam);
	}

	prgxp GetFirstChildByName(const std::wstring& Name, unsigned long nOffset) const
	{
		IXParamPtr spXParam;
		m_spInterface->GetFirstChildByName(Name, nOffset, spXParam.getaddress());
		return prgxp(spXParam);
	}

	prgxp GetLastChild() const
	{
		IXParamPtr spXParam;
		m_spInterface->GetLastChild(spXParam.getaddress());
		return prgxp(spXParam);
	}

	prgxp GetLastChildByName(const std::wstring& Name, unsigned long nOffset) const
	{
		IXParamPtr spXParam;
		m_spInterface->GetLastChildByName(Name, nOffset, spXParam.getaddress());
		return prgxp(spXParam);
	}

	unsigned long GetChildrenCount() const
	{
		unsigned long nCount = 0;
		m_spInterface->GetChildrenCount(&nCount);
		return nCount;
	}

	prgxp GetPrevSibling() const
	{
		IXParamPtr spXParam;
		m_spInterface->GetPrevSibling(spXParam.getaddress());
		return prgxp(spXParam);
	}

	prgxp GetPrevNamesake() const
	{
		IXParamPtr spXParam;
		m_spInterface->GetPrevNamesake(spXParam.getaddress());
		return prgxp(spXParam);
	}

	prgxp GetNextSibling() const
	{
		IXParamPtr spXParam;
		m_spInterface->GetNextSibling(spXParam.getaddress());
		return prgxp(spXParam);
	}

	prgxp GetNextNamesake() const
	{
		IXParamPtr spXParam;
		m_spInterface->GetNextNamesake(spXParam.getaddress());
		return prgxp(spXParam);
	}

	std::wstring GetName() const
	{
		std::wstring strName;
		m_spInterface->GetName(strName);
		return strName;
	}

	HRESULT SetName(const std::wstring& Name) const
	{
		return m_spInterface->SetName(Name);
	}

	HRESULT LockIt() const
	{
		return m_spInterface->LockIt();
	}

	HRESULT Unlockit() const
	{
		return m_spInterface->UnlockIt();
	}

	HRESULT FromXmlString(const std::wstring& XmlString) const
	{
		return m_spInterface->FromXmlString(XmlString);
	}

	std::wstring ToXmlString(const std::wstring& Indent, const std::wstring& crlf) const
	{
		std::wstring XmlString;
		m_spInterface->ToXmlString(Indent, crlf, XmlString);
		return XmlString;
	}

    HRESULT FromXmlFile(const base::FilePath &XmlFilePath) const
    {
        return m_spInterface->FromXmlFile(XmlFilePath);
    }

    HRESULT ToXmlFile(const base::FilePath &XmlFilePath, const std::wstring& Indent, const std::wstring& crlf, const std::wstring& Encoding) const
    {
        return m_spInterface->ToXmlFile(XmlFilePath, Indent, crlf, Encoding);
    }

	HRESULT FromXmlMemory(void* XmlMemory, unsigned int XmlSize) const
	{
		return m_spInterface->FromXmlMemory(XmlMemory, XmlSize);
	}
};

struct XParamScopeLock
{
    XParamScopeLock(IXParam* pXParam);
    ~XParamScopeLock();
    
	IXParamPtr m_spInterface;
};

inline XParamScopeLock::XParamScopeLock(IXParam* pXParam)
: m_spInterface(pXParam)
{
    m_spInterface->LockIt();
};
inline XParamScopeLock::~XParamScopeLock()
{
    m_spInterface->UnlockIt();
};

template <typename T>
inline HRESULT XParamToClass(IXParam* pXParam, T const&t)
{
	if (!pXParam)
		return E_POINTER;

	XParamScopeLock lock(pXParam);
	HRESULT hr = S_OK;
	xparam::XParam* pCppXParam = NULL;
	hr = pXParam->GetCppXParam((void**)&pCppXParam);
	if (FAILED(hr)) return hr;
	xparam::XParamP spCppXParam(pCppXParam, false);
	srlz::iarchive_xparam(spCppXParam) >> const_cast<T&>(t);
	return hr;
}

template <typename T>
inline HRESULT XParamFromClass(IXParam** ppXParam, T const& t)
{
	HRESULT hr = S_OK;

	hr = ::CreateXParam(ppXParam);
	if (FAILED(hr)) return hr;

	xparam::XParam* pCppRoot = NULL;
	hr = (*ppXParam)->GetCppXParam((void**)&pCppRoot);
	if (FAILED(hr)) return hr;
	xparam::XParamP spCppRoot(pCppRoot, false);
	srlz::oarchive_xparam(L"", spCppRoot) << t;
	(*ppXParam)->SetCppXParam(spCppRoot->GetDocument().get());
	return hr;
}


template <typename T>
inline IXParamPtr XParamFromClass(T const& t)
{
	IXParamPtr spResult;
	XParamFromClass(spResult.getaddress(), t);
	return spResult;
}

END_NAMESPACE()

#endif //__XParamHelper_H_15C73818_8FD8_4B5F_A3BA_9971D11CE2EC_INCLUDED__
