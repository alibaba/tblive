#ifndef __ARCHIVE_XPARAM_BASIC_H_AD81B9F0_043C_46DE_861A_A5C30F60D657__
#define __ARCHIVE_XPARAM_BASIC_H_AD81B9F0_043C_46DE_861A_A5C30F60D657__

#include "./srlz.h"
#include "./archive_pchar.h"
#include "./xml_addons.h"

#include <base/xparam/XParamCore.h>
#include <typeinfo>
#include <type_traits>

#include "base/srlz/template_util.h"


NS_SERIALIZATION_LITE_BEGIN

class xparam_standin;

class xparam_standin : public standin
{
public:
    xparam_standin(xparam::XParamP const& node);
    xparam_standin(xparam_standin const& src);
    virtual ~xparam_standin();
	inline static const std::wstring static_type() {return L"xparam_standin{477E5272-34DF-42ef-8955-6A56D429185A}";}
    virtual const std::wstring type() const OVERRIDE;
	xparam::XParamP m_node;
};
inline xparam_standin::xparam_standin(xparam::XParamP const& node) : m_node(node){}
inline xparam_standin::xparam_standin(xparam_standin const& src) : m_node(src.m_node){}
inline xparam_standin::~xparam_standin(){}
inline const std::wstring xparam_standin::type() const {return static_type();}


//////////////////////////////////////////////////////////////////////////
// archives 定义

//////////////////////////////////////////////////////////////////////////
// class iarchive_xparam
class iarchive_xparam : public iarchive_base<iarchive_xparam>
{
public:
    iarchive_xparam(xparam::XParamP const& element, bool wildmatch = false);
    iarchive_xparam(const iarchive_xparam& src);
    ~iarchive_xparam();

	inline bool exists() const
	{
		return m_element.get() ? true : false;
	}
	inline wchar_t const* name(size_t* nlen, xparam::XInResP& inres) const
	{
		if (exists())
		{
			wchar_t const* n = NULL;
			size_t len;
			m_element->GetName(n, len, inres);
			if (nlen)
				*nlen = len;
			return n;
		}
		if (nlen)
			*nlen = 0;
		return NULL;
	}
	inline bool equal_name(wchar_t const* n) const
	{
		if (exists())
		{
			wchar_t const* nn = NULL;
			size_t nnlen;
			xparam::XInResP inres;
			bool ret = m_element->GetName(nn, nnlen, inres);
			if (!ret || !n || !nn)
				return false;
			wchar_t const* end = nn + nnlen;
			for (; nn < end && *n; ++nn, ++n)
			{
				if (*n != *nn)
					return false;
			}
			return nn == end && *n == 0;
		}
		return false;
	}
	template<typename T>
	inline bool value(T& t) const
	{
		return exists() && m_element->GetValue(t);
	}
	inline bool value(wchar_t const*& v, size_t* vlen, xparam::XInResP& inres) const
	{
		if (exists())
		{
			size_t len;
			bool ret = m_element->GetValue(v, len, inres);
			if (ret && vlen)
				*vlen = len;
			return ret;
		}
		return false;
	}
    inline bool value(char const*& v, size_t* vlen, xparam::XInResP& inres) const
    {
        if (exists())
        {
            size_t len;
            bool ret = m_element->GetValue(v, len, inres);
            if (ret && vlen)
                *vlen = len;
            return ret;
        }
        return false;
    }
	inline bool equal_value(wchar_t const* v) const
	{
		if (exists())
		{
			wchar_t const* vv = NULL;
			size_t vvlen;
			xparam::XInResP inres;
			bool ret = m_element->GetValue(vv, vvlen, inres);
			if (!ret || !v || !vv)
				return false;
			wchar_t const* end = vv + vvlen;
			for (; vv < end && *v; ++vv, ++v)
			{
				if (*v != *vv)
					return false;
			}
			return vv == end && *v == 0;
		}
		return false;
	}
	inline bool value(void const*& v, size_t* vlen, xparam::XInResP& inres) const
	{
		if (exists())
		{
			size_t len;
			bool ret = m_element->GetValue(v, len, inres);
			if (ret && vlen)
				*vlen = len;
			return ret;
		}
		return false;
	}
	inline iarchive_xparam first_child(wchar_t const* name, size_t nlen) const
	{
		if (!exists())
			return iarchive_xparam(xparam::XParamP());
		if (nlen ==0 || !name || !name[0])
			return iarchive_xparam(m_element->GetFirstChild(), true);
		return iarchive_xparam(m_element->GetFirstChild(name, nlen, 0));
	}
	inline iarchive_xparam next_namesake() const
	{
		if (!exists())
			return iarchive_xparam(xparam::XParamP());
		if (m_wildmatch)
			return iarchive_xparam(m_element->GetNextSibling(), true);
		return iarchive_xparam(m_element->GetNextNamesake());
	}
//	inline void to_standin(boost::shared_ptr<standin>& t) const
//	{
//		if (exists())
//			t.reset(new xparam_standin(m_element));
//	}
private:
	xparam::XParamP m_element;
	bool m_wildmatch;
};

inline iarchive_xparam::iarchive_xparam(xparam::XParamP const& element, bool wildmatch)
: m_element(element)
, m_wildmatch(wildmatch)
{
}
inline iarchive_xparam::iarchive_xparam(const iarchive_xparam& src)
: m_element(src.m_element)
, m_wildmatch(src.m_wildmatch)
{
}
inline iarchive_xparam::~iarchive_xparam(){}

//////////////////////////////////////////////////////////////////////////
// class oarchive_xparam
class oarchive_xparam : public oarchive_base<oarchive_xparam>
{
public:
    oarchive_xparam(wchar_t const* name, xparam::XParamP const& parent, xparam::X_VALUE_NODE value_ntype = xparam::xvn_text);
    oarchive_xparam(oarchive_xparam const& src);
    ~oarchive_xparam();
	inline wchar_t const* name()
	{
		return m_name;
	}
	inline void name(wchar_t const* n)
	{
		m_name = n;
	}
	template<typename T>
	inline void value(T const& t)
	{
		ensure();
		m_element->SetValue(t);
	}
	inline void value(wchar_t const* v, size_t vlen)
	{
		ensure();
		m_element->SetValue(v, vlen);
	}
    inline void value(char const* v, size_t vlen)
    {
        ensure();
        m_element->SetValue(v, vlen);
    }
	inline void value(void const* v, size_t vlen)
	{
		ensure();
		m_element->SetValue(v, vlen);
	}
	inline oarchive_xparam first_child(wchar_t const* n, xparam::X_VALUE_NODE value_ntype)
	{
		ensure();
		return oarchive_xparam(n, m_element, value_ntype);
	}
	inline oarchive_xparam next_sibling(wchar_t const* n)
	{
		ensure();
		return oarchive_xparam(n, m_parent, m_value_nodetype);
	}
	inline bool exists_child(wchar_t const* n) const
	{
		return (m_element && m_element->GetFirstChild(n, xparam::ZERO_TERMINATOR));
	}

//	inline void from_standin(boost::shared_ptr<standin> const& t)
//	{
//		boost::shared_ptr<xparam_standin> xsi = standin_cast<xparam_standin>(t);
//		if (xsi)
//		{
//			if (m_element)
//				m_parent->DropChild(m_element);
//			m_parent->PushBackChild(xsi->m_node);
//		}
//	}
	inline xparam::XParamP get() const
	{
		return m_element;
	}
private:
	inline void ensure()
	{
		if (!m_element)
		{
			m_element = m_parent->CreateXParam();
			m_element->SetName(m_name, xparam::ZERO_TERMINATOR);
			m_element->SetValueNode(m_value_nodetype);
			m_parent->PushBackChild(m_element);
		}
	}
	xparam::XParamP m_parent;
	xparam::XParamP m_element;
	wchar_t const* m_name;
	xparam::X_VALUE_NODE m_value_nodetype;
};

inline oarchive_xparam::oarchive_xparam(wchar_t const* name, xparam::XParamP const& parent, xparam::X_VALUE_NODE value_ntype)
: m_parent(parent)
, m_name(name)
, m_value_nodetype(value_ntype)
{
}
inline oarchive_xparam::oarchive_xparam(oarchive_xparam const& src)
: m_parent(src.m_parent)
, m_element(src.m_element)
, m_name(src.m_name)
, m_value_nodetype(src.m_value_nodetype)
{
}
inline oarchive_xparam::~oarchive_xparam(){}


//////////////////////////////////////////////////////////////////////////
// 序列化方法


//////////////////////////////////////////////////////////////////////////
// member_nvio的序列化
namespace detail
{
	// 使用模版特化来完成addon中的节点类型匹配
	template <typename T>
	struct xparam_get_value_node;

	template <>
	struct xparam_get_value_node< ::srlz::addon::xml_mapping_nodetype::xmn_text>
	{
		inline static xparam::X_VALUE_NODE get() {return xparam::xvn_text;}
	};

	template <>
	struct xparam_get_value_node< ::srlz::addon::xml_mapping_nodetype::xmn_cdata>
	{
		inline static xparam::X_VALUE_NODE get() {return xparam::xvn_cdata;}
	};

	template <>
	struct xparam_get_value_node< ::srlz::addon::xml_mapping_nodetype::xmn_attr>
	{
		inline static xparam::X_VALUE_NODE get() {return xparam::xvn_attr;}
	};

}

template <typename OwnerType, size_t offset, typename CharType, typename ValueType, typename InitType>
inline void serialize_load(const iarchive_xparam& ar, member_nvio<OwnerType, offset, CharType, ValueType, InitType>& nvio)
{
	if (nvio.name == NULL)
		ar >> nvio.value;
	else
	{
		iarchive_xparam child = ar.first_child(nvio.name, xparam::ZERO_TERMINATOR);
		if (child.exists())
			child >> nvio.value;
	}
}

template <typename OwnerType, size_t offset, typename CharType, typename ValueType, typename InitType>
inline void serialize_save(oarchive_xparam& ar, member_nvio<OwnerType, offset, CharType, ValueType, InitType> const& nvio)
{
	if (nvio.name == NULL)
		ar << nvio.value;
	else
	{
		ar.first_child(nvio.name, detail::xparam_get_value_node<typename addon::xml_mapping<OwnerType, offset>::nodetype>::get()) << nvio.value;
	}
}

//////////////////////////////////////////////////////////////////////////
// standin的序列化
//inline void serialize_load(iarchive_xparam const& ar, boost::shared_ptr<standin>& t)
//{
//	ar.to_standin(t);
//}
//
//inline void serialize_save(oarchive_xparam& ar, boost::shared_ptr<standin> const& t)
//{
//	ar.from_standin(t);
//}

//////////////////////////////////////////////////////////////////////////
// 字符数组序列化
template <typename CharType, size_t array_size>
inline void serialize_load(iarchive_xparam const& ar, CharType (&t)[array_size])
{
	if (ar.exists())
	{
		size_t vlen;
		xparam::XInResP inres;
		CharType const* v;
		ar.value(v, &vlen, inres);
		iarchive_pchar<CharType>(v, vlen) >> t;
	}
}

template <typename CharType, size_t array_size>
inline void serialize_save(oarchive_xparam& ar, CharType const (&t)[array_size])
{
	oarchive_pchar<CharType> opc;
	opc << t;
	ar.value(opc.pchar, opc.len);
}

//////////////////////////////////////////////////////////////////////////
// std::basic_string序列化
template <typename CharType, typename _Traits, typename _Ax>
inline void serialize_load(iarchive_xparam const& ar, std::basic_string<CharType, _Traits, _Ax>& t)
{
	if (ar.exists())
	{
		CharType const* v = {0};
		size_t vlen = 0;
		xparam::XInResP inres;
		if (ar.value(v, &vlen, inres))
			t.assign(v, vlen);
	}
}

template <typename CharType, typename _Traits, typename _Ax>
inline void serialize_save(oarchive_xparam& ar, std::basic_string<CharType, _Traits, _Ax> const& t)
{
	ar.value(t.c_str(), t.length());
}

//////////////////////////////////////////////////////////////////////////
// std::basic_string<unsigned char>的序列化，作为blob
template <typename _Traits, typename _Ax>
inline void serialize_load(iarchive_xparam const& ar, std::basic_string<unsigned char, _Traits, _Ax>& t)
{
	if (ar.exists())
	{
		void const* v = {0};
		size_t vlen = 0;
		xparam::XInResP inres;
		if (ar.value(v, &vlen, inres))
			t.assign((unsigned char const*)v, vlen);
	}
}

template <typename _Traits, typename _Ax>
inline void serialize_save(oarchive_xparam& ar, std::basic_string<unsigned char, _Traits, _Ax> const& t)
{
	ar.value((void const*)t.c_str(), t.length());
}

//////////////////////////////////////////////////////////////////////////
// 指针类型的序列化，支持多态的方式使用
template <typename T>
struct serialize_loadptr_executor<iarchive_xparam, T>
{
	template <typename Base>
	inline static bool exec(iarchive_xparam const& ar, Base*& p, void*)
	{
		if (!p)
			p = ptr_creator<T>::create();
		ar >> *p;
		return true;
	}

	template <typename Base, typename CharType>
	inline static bool exec(iarchive_xparam const& ar, Base*& p, CharType const* clsname)
	{
		bool matched = false;
		if (addon::xml_inherited_tag<Base>::type() == addon::xitt_self)
			matched = ar.equal_name(clsname);
		else
		{
			iarchive_xparam ch = ar.first_child(addon::xml_inherited_tag<Base>::name((CharType)0), -1);
			matched = ch.exists() && ch.equal_value(clsname);
		}
		if (matched)
		{
			if (!p || static_cast<T*>(p) != NULL)
				p = ptr_creator<T>::create();
			ar >> *static_cast<T*>(p);
			return true;
		}
		return false;
	}
};

template <typename T>
struct serialize_saveptr_executor<oarchive_xparam, T>
{
	template <typename Base>
	inline static bool exec(oarchive_xparam& ar, Base*const& p, void*)
	{
		if (p)
			ar << *p;
		return true;
	}
	template <typename Base, typename CharType>
	inline static bool exec(oarchive_xparam& ar, Base*const& p, CharType const* clsname)
	{
		if (p && static_cast<T*>(p) != NULL) // 使用type_info比较和static_cast性能上应该比dynamic_cast好，并且有静态类型检查
		{
			T* pt = static_cast<T*>(p);
			if (clsname)
			{
				if (addon::xml_inherited_tag<Base>::type() == addon::xitt_self)
				{
					ar.name(clsname);
					ar << *pt;
				}
				else if (addon::xml_inherited_tag<Base>::type() == addon::xitt_attr)
				{
					ar << *pt;
					ar.first_child(addon::xml_inherited_tag<Base>::name((CharType)0), xparam::xvn_attr) << std::basic_string<CharType>(clsname);
				}
				else
				{
					ar << *pt;
					ar.first_child(addon::xml_inherited_tag<Base>::name((CharType)0), xparam::xvn_text) << std::basic_string<CharType>(clsname);
				}
			}
			else
				ar << *pt;
			return true;
		}
		return false;
	}
};

//////////////////////////////////////////////////////////////////////////
// 基础类型的序列化

#define XPARAM_TRANSLATE_BASIC_TYPE(type) \
inline void serialize_load(iarchive_xparam const& ar, type& t)\
{\
	ar.value(t);\
}\
inline void serialize_save(oarchive_xparam& ar, type const& t)\
{\
	ar.value(t);\
}\


XPARAM_TRANSLATE_BASIC_TYPE(char)
XPARAM_TRANSLATE_BASIC_TYPE(unsigned char)
XPARAM_TRANSLATE_BASIC_TYPE(short)
XPARAM_TRANSLATE_BASIC_TYPE(unsigned short)
XPARAM_TRANSLATE_BASIC_TYPE(long)
XPARAM_TRANSLATE_BASIC_TYPE(unsigned long)
XPARAM_TRANSLATE_BASIC_TYPE(long long)
XPARAM_TRANSLATE_BASIC_TYPE(unsigned long long)
XPARAM_TRANSLATE_BASIC_TYPE(int)
XPARAM_TRANSLATE_BASIC_TYPE(unsigned int)

XPARAM_TRANSLATE_BASIC_TYPE(float)
XPARAM_TRANSLATE_BASIC_TYPE(double)
XPARAM_TRANSLATE_BASIC_TYPE(long double)

XPARAM_TRANSLATE_BASIC_TYPE(bool)
XPARAM_TRANSLATE_BASIC_TYPE(void*)
//XPARAM_TRANSLATE_BASIC_TYPE(boost::shared_ptr<void>)
#ifdef WIN32
XPARAM_TRANSLATE_BASIC_TYPE(IUnknown*)
XPARAM_TRANSLATE_BASIC_TYPE(IDispatch*)
#endif

#undef XPARAM_TRANSLATE_BASIC_TYPE


// enum 
template<typename EnumType>
inline typename srlz::internal::enable_if_c< std::is_enum<EnumType>::value >::type
serialize_load(iarchive_xparam const& ar, EnumType& t)
{
	int t_ = 0;
	ar.value(t_);
	t = (EnumType)t_;
}

template<typename EnumType>
inline typename srlz::internal::enable_if_c< std::is_enum<EnumType>::value >::type
serialize_save(oarchive_xparam& ar, EnumType const& t)
{
	ar.value((int)t);
}

NS_SERIALIZATION_LITE_END

#endif//__ARCHIVE_XPARAM_BASIC_H_AD81B9F0_043C_46DE_861A_A5C30F60D657__

