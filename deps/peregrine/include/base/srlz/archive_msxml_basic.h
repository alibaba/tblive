#ifndef __ARCHIVE_MSXML_BASIC_H_AD81B9F0_043C_46DE_861A_A5C30F60D657__
#define __ARCHIVE_MSXML_BASIC_H_AD81B9F0_043C_46DE_861A_A5C30F60D657__

#include "./srlz.h"
#include "./archive_pchar.h"
#include "./xml_addons.h"
#include <comdef.h>
#include <msxml2.h>
#include <comdefsp.h>
#include <oleauto.h>
#include <boost/smart_ptr.hpp>
#include <typeinfo>

NS_SERIALIZATION_LITE_BEGIN


class msxml_standin : public standin
{
public:
	inline msxml_standin(IXMLDOMNodePtr const& node) : m_node(node){}
	inline msxml_standin(msxml_standin const& src) : m_node(src.m_node){}
	inline static const char* static_type() {return "msxml_w_standin{EFBFD911-2475-45f6-B1D9-AE978F246CAB}";}
	virtual const char* type() const {return static_type();}
	IXMLDOMNodePtr m_node;
};

//////////////////////////////////////////////////////////////////////////
// archives 定义

//////////////////////////////////////////////////////////////////////////
// iarchive_msxml_attr
// 
class iarchive_msxml_attr : public iarchive_base<iarchive_msxml_attr>
{
public:
	inline iarchive_msxml_attr(IXMLDOMAttributePtr const& attr)
		: m_attr(attr)
	{
	}
	inline iarchive_msxml_attr(const iarchive_msxml_attr& src)
		: m_attr(src.m_attr)
	{
	}
	inline bool exists() const
	{
		return m_attr != NULL;
	}
	inline _bstr_t name() const
	{
		_bstr_t n;
		if (exists())
			m_attr->get_name(n.GetAddress());
		return n;
	}
	inline _bstr_t value() const
	{
		_bstr_t v;
		if (exists())
			m_attr->get_text(v.GetAddress());
		return v;
	}
	inline bool equal_value(_bstr_t const& v) const
	{
		if (exists())
		{
			_bstr_t vv;
			m_attr->get_text(vv.GetAddress());
			return vv == v;
		}
		return false;
	}
	inline iarchive_msxml_attr next_sibling() const
	{
		IXMLDOMNodePtr p = m_attr;
		while (p != NULL)
		{
			IXMLDOMNodePtr q;
			DOMNodeType ntype;
			if (SUCCEEDED(p->get_nextSibling(&q)) && q != NULL && SUCCEEDED(q->get_nodeType(&ntype)) && ntype == NODE_ATTRIBUTE)
				return (iarchive_msxml_attr(q));
			p = q;
		}
		return iarchive_msxml_attr(p);
	}

private:
	IXMLDOMAttributePtr m_attr;
};

//////////////////////////////////////////////////////////////////////////
// class oarchive_msxml_attr
class oarchive_msxml_attr : public oarchive_base<oarchive_msxml_attr>
{
public:
	inline oarchive_msxml_attr(_bstr_t const& name, IXMLDOMElementPtr const& parent)
	: m_parent(parent)
	, m_name(name)
	{
	}
	inline oarchive_msxml_attr(oarchive_msxml_attr const& src)
	: m_parent(src.m_parent)
	, m_attr(src.m_attr)
	, m_name(src.m_name)
	{
	}
	inline _bstr_t const& name()
	{
		return m_name;
	}
	inline void name(_bstr_t const& n)
	{
		m_name = n;
	}
	inline void value(_bstr_t const& v)
	{
		ensure();
		m_attr->put_text(v);
	}
	inline oarchive_msxml_attr next_sibling(_bstr_t const& name)
	{
		ensure();
		return oarchive_msxml_attr(name, m_parent);
	}	
private:
	inline void ensure()
	{
		if (m_attr == NULL)
		{
			m_parent->setAttribute(m_name, _variant_t());
			m_parent->getAttributeNode(m_name, &m_attr);
		}
	}
	IXMLDOMElementPtr m_parent;
	IXMLDOMAttributePtr m_attr;
	_bstr_t m_name;
};


//////////////////////////////////////////////////////////////////////////
// class iarchive_msxml
class iarchive_msxml : public iarchive_base<iarchive_msxml>
{
public:
	inline iarchive_msxml(IXMLDOMElementPtr const& element, bool wildmatch = false)
	: m_element(element)
	, m_wildmatch(wildmatch)
	{
	}
	inline iarchive_msxml(const iarchive_msxml& src)
	: m_element(src.m_element)
	, m_wildmatch(src.m_wildmatch)
	{
	}
	inline iarchive_msxml(const boost::shared_ptr<standin>& src)
		: m_wildmatch(false)
	{
		boost::shared_ptr<msxml_standin> s = standin_cast<msxml_standin>(src);
		if (s)
			m_element = s->m_node;
	}
	inline bool exists() const
	{
		return m_element != NULL;
	}
	inline _bstr_t name() const
	{
		_bstr_t n;
		if (exists())
			m_element->get_nodeName(n.GetAddress());
		return n;
	}
	inline bool equal_name(_bstr_t const& n) const
	{
		if (exists())
		{
			_bstr_t nn;
			m_element->get_nodeName(nn.GetAddress());
			return nn == n;
		}
		return false;
	}
	inline _bstr_t value() const
	{
		_bstr_t v;
		if (exists())
			m_element->get_text(v.GetAddress());
		return v;
	}
	inline bool equal_value(_bstr_t const& v) const
	{
		if (exists())
		{
			_bstr_t vv;
			m_element->get_text(vv.GetAddress());
			return vv == v;
		}
		return false;
	}
	inline iarchive_msxml next_sibling(_bstr_t const& name) const
	{
		IXMLDOMNodePtr p = m_element;
		if (name.length() == 0)
		{
			while (p != NULL)
			{
				IXMLDOMNodePtr q;
				DOMNodeType ntype;
				_bstr_t n;
				if (SUCCEEDED(p->get_nextSibling(&q)) && 
					q != NULL && 
					SUCCEEDED(q->get_nodeType(&ntype)) && 
					ntype == NODE_ELEMENT)
					return (iarchive_msxml(q));
				p = q;
			}
			return iarchive_msxml(IXMLDOMElementPtr());
		}
		while (p != NULL)
		{
			IXMLDOMNodePtr q;
			DOMNodeType ntype;
			_bstr_t n;
			if (SUCCEEDED(p->get_nextSibling(&q)) && 
				q != NULL && 
				SUCCEEDED(q->get_nodeType(&ntype)) && 
				ntype == NODE_ELEMENT &&
				SUCCEEDED(q->get_nodeName(n.GetAddress())) &&
				name == n)
				return (iarchive_msxml(q));
			p = q;
		}
		return iarchive_msxml(IXMLDOMElementPtr());
	}
	inline iarchive_msxml first_child(_bstr_t const& name) const
	{
		IXMLDOMNodePtr p;
		if (m_element == NULL || FAILED(m_element->get_firstChild(&p)) || p == NULL)
			return iarchive_msxml(IXMLDOMElementPtr());
		if (name.length() == 0)
		{
			do
			{
				DOMNodeType ntype;
				if (SUCCEEDED(p->get_nodeType(&ntype)) && ntype == NODE_ELEMENT)
					return iarchive_msxml(p);
				IXMLDOMNodePtr q;
				p->get_nextSibling(&q);
				p = q;
			}while(p != NULL);
			return iarchive_msxml(IXMLDOMElementPtr());
		}
		do
		{
			DOMNodeType ntype;
			_bstr_t n;
			if (SUCCEEDED(p->get_nodeType(&ntype)) && 
				ntype == NODE_ELEMENT &&
				SUCCEEDED(p->get_nodeName(n.GetAddress())) &&
				name == n)
				return iarchive_msxml(p);
			IXMLDOMNodePtr q;
			p->get_nextSibling(&q);
			p = q;
		}while(p != NULL);
		return iarchive_msxml(IXMLDOMElementPtr());
	}
	inline iarchive_msxml_attr attr(_bstr_t const& name) const
	{
		IXMLDOMAttributePtr p;
		if (exists())
			m_element->getAttributeNode(name, &p);
		return p;
	}
	inline void to_standin(boost::shared_ptr<standin>& t) const
	{
		if (exists())
			t.reset(new msxml_standin(m_element));
	}
private:
	IXMLDOMElementPtr m_element;
	bool m_wildmatch;
};

//////////////////////////////////////////////////////////////////////////
// class oarchive_msxml
class oarchive_msxml : public oarchive_base<oarchive_msxml>
{
public:
	inline oarchive_msxml(_bstr_t const& name, IXMLDOMElementPtr const& parent, DOMNodeType value_ntype = NODE_TEXT)
	: m_parent(parent)
	, m_name(name)
	, m_value_nodetype(value_ntype)
	{
	}
	inline oarchive_msxml(oarchive_msxml const& src)
	: m_parent(src.m_parent)
	, m_element(src.m_element)
	, m_name(src.m_name)
	, m_value_nodetype(src.m_value_nodetype)
	{
	}
	inline _bstr_t const& name()
	{
		return m_name;
	}
	inline void name(_bstr_t const& n)
	{
		m_name = n;
	}
	inline void value(_bstr_t const& v)
	{
		ensure();
		m_element->put_text(v);
	}
	inline oarchive_msxml first_child(_bstr_t const& name, DOMNodeType value_ntype)
	{
		ensure();
		return oarchive_msxml(name, m_element, value_ntype);
	}
	inline oarchive_msxml next_sibling(_bstr_t const& name)
	{
		ensure();
		return oarchive_msxml(name, m_parent, m_value_nodetype);
	}
	inline oarchive_msxml_attr attr(_bstr_t const& name)
	{
		ensure();
		return oarchive_msxml_attr(name, m_element);
	}
	inline void from_standin(boost::shared_ptr<standin> const& t)
	{
		boost::shared_ptr<msxml_standin> xsi = standin_cast<msxml_standin>(t);
		if (xsi)
		{
			IXMLDOMNodePtr p;
			m_parent->removeChild(m_element, &p);
			m_parent->appendChild(xsi->m_node, &p);
		}
	}
	inline IXMLDOMElementPtr get() const
	{
		return m_element;
	}
private:
	inline void ensure()
	{
		if (m_element == NULL)
		{
			IXMLDOMNodePtr p;
			IXMLDOMDocumentPtr doc;
			m_parent->get_ownerDocument(&doc);
			doc->createElement(m_name, &m_element);
			m_parent->appendChild(m_element, &p);
		}
	}
	IXMLDOMElementPtr m_parent;
	IXMLDOMElementPtr m_element;
	_bstr_t m_name;
	DOMNodeType m_value_nodetype;
};


//////////////////////////////////////////////////////////////////////////
// 序列化方法


//////////////////////////////////////////////////////////////////////////
// member_nvio的序列化
namespace detail
{
	// 使用模版特化来完成addon中的节点类型匹配
	template <typename T> struct msxml_nvio_serializer_;

	template <>
	struct msxml_nvio_serializer_<addon::xml_mapping_nodetype::xmn_attr>
	{
		template <typename OwnerType, size_t offset, typename CharType, typename ValueType, typename InitType>
		inline static void load(const iarchive_msxml& ar, member_nvio<OwnerType, offset, CharType, ValueType, InitType>& nvio)
		{
			iarchive_msxml_attr attr = ar.attr(nvio.name);
			if (attr.exists())
				attr >> nvio.value;
			else
			{
				iarchive_msxml child = ar.first_child(nvio.name);
				if (child.exists())
					child >> nvio.value;
			}
		}
		template <typename OwnerType, size_t offset, typename CharType, typename ValueType, typename InitType>
		inline static void save(oarchive_msxml& ar, member_nvio<OwnerType, offset, CharType, ValueType, InitType> const& nvio)
		{
			ar.attr(nvio.name) << nvio.value;
		}
	};
	template <>
	struct msxml_nvio_serializer_<addon::xml_mapping_nodetype::xmn_text>
	{
		template <typename OwnerType, size_t offset, typename CharType, typename ValueType, typename InitType>
		inline static void load(const iarchive_msxml& ar, member_nvio<OwnerType, offset, CharType, ValueType, InitType>& nvio)
		{
			iarchive_msxml child = ar.first_child(nvio.name);
			if (child.exists())
				child >> nvio.value;
			else
			{
				iarchive_msxml_attr attr = ar.attr(nvio.name);
				if (attr.exists())
					attr >> nvio.value;
			}
		}
		template <typename OwnerType, size_t offset, typename CharType, typename ValueType, typename InitType>
		inline static void save(oarchive_msxml& ar, member_nvio<OwnerType, offset, CharType, ValueType, InitType> const& nvio)
		{
			ar.first_child(nvio.name, NODE_TEXT) << nvio.value;
		}
	};
	template <>
	struct msxml_nvio_serializer_<addon::xml_mapping_nodetype::xmn_cdata>
	{
		template <typename OwnerType, size_t offset, typename CharType, typename ValueType, typename InitType>
		inline static void load(const iarchive_msxml& ar, member_nvio<OwnerType, offset, CharType, ValueType, InitType>& nvio)
		{
			iarchive_msxml child = ar.first_child(nvio.name);
			if (child.exists())
				child >> nvio.value;
			else
			{
				iarchive_msxml_attr attr = ar.attr(nvio.name);
				if (attr.exists())
					attr >> nvio.value;
			}
		}
		template <typename OwnerType, size_t offset, typename CharType, typename ValueType, typename InitType>
		inline static void save(oarchive_msxml& ar, member_nvio<OwnerType, offset, CharType, ValueType, InitType> const& nvio)
		{
			ar.first_child(nvio.name, NODE_CDATA_SECTION) << nvio.value;
		}
	};
}

template <typename OwnerType, size_t offset, typename CharType, typename ValueType, typename InitType>
inline void serialize_load(const iarchive_msxml& ar, member_nvio<OwnerType, offset, CharType, ValueType, InitType>& nvio)
{
	if (nvio.name == NULL)
		ar >> nvio.value;
	else
		detail::msxml_nvio_serializer_<addon::xml_mapping<OwnerType, offset>::nodetype>::load(ar, nvio);
}

template <typename OwnerType, size_t offset, typename CharType, typename ValueType, typename InitType>
inline void serialize_load(const iarchive_msxml_attr& ar, member_nvio<OwnerType, offset, CharType, ValueType, InitType>& nvio)
{
	if (ar.exists())
		ar >> nvio.value;
}

template <typename OwnerType, size_t offset, typename CharType, typename ValueType, typename InitType>
inline void serialize_save(oarchive_msxml& ar, member_nvio<OwnerType, offset, CharType, ValueType, InitType> const& nvio)
{
	if (nvio.name == NULL)
		ar << nvio.value;
	else
		detail::msxml_nvio_serializer_<addon::xml_mapping<OwnerType, offset>::nodetype>::save(ar, nvio);
}

template <typename OwnerType, size_t offset, typename CharType, typename ValueType, typename InitType>
inline void serialize_save(oarchive_msxml_attr& ar, member_nvio<OwnerType, offset, CharType, ValueType, InitType> const& nvio)
{
	ar << nvio.value;
}

//////////////////////////////////////////////////////////////////////////
// standin的序列化
inline void serialize_load(iarchive_msxml const& ar, boost::shared_ptr<standin>& t)
{
	ar.to_standin(t);
}

inline void serialize_load(iarchive_msxml_attr const& ar, boost::shared_ptr<standin>& t)
{
	// NULL
}

inline void serialize_save(oarchive_msxml& ar, boost::shared_ptr<standin> const& t)
{
	ar.from_standin(t);
}

inline void serialize_save(oarchive_msxml_attr& ar, boost::shared_ptr<standin> const& t)
{
	// NULL
}

//////////////////////////////////////////////////////////////////////////
// 字符数组序列化
template <typename CharType, size_t array_size>
inline void serialize_load(iarchive_msxml const& ar, CharType (&t)[array_size])
{
	if (ar.exists())
	{
		_bstr_t bstr = ar.value();
		size_t len = sizeof(CharType) == sizeof(char) ? ZERO_TERM : bstr.length();
		iarchive_pchar<CharType>(bstr, len) >> t;
	}
}

template <typename CharType, size_t array_size>
inline void serialize_load(iarchive_msxml_attr const& ar, CharType (&t)[array_size])
{
	if (ar.exists())
	{
		_bstr_t bstr = ar.value();
		size_t len = sizeof(CharType) == sizeof(char) ? ZERO_TERM : bstr.length();
		iarchive_pchar<CharType>(bstr, len) >> t;
	}
}

template <typename CharType, size_t array_size>
inline void serialize_save(oarchive_msxml& ar, CharType const (&t)[array_size])
{
	oarchive_pchar<CharType> opc;
	opc << t;
	ar.value(opc.pchar);
}

template <typename CharType, size_t array_size>
inline void serialize_save(oarchive_msxml_attr& ar, CharType const (&t)[array_size])
{
	oarchive_pchar<CharType> opc;
	opc << t;
	ar.value(opc.pchar);
}

//////////////////////////////////////////////////////////////////////////
// std::basic_string序列化

template <typename CharType, typename _Traits, typename _Ax>
inline void serialize_load(iarchive_msxml const& ar, std::basic_string<CharType, _Traits, _Ax>& t)
{
	if (ar.exists())
	{
		_bstr_t bstr = ar.value();
		size_t len = sizeof(CharType) == sizeof(char) ? ZERO_TERM : bstr.length();
		iarchive_pchar<CharType>(bstr, len) >> t;
	}
}

template <typename CharType, typename _Traits, typename _Ax>
inline void serialize_load(iarchive_msxml_attr const& ar, std::basic_string<CharType, _Traits, _Ax>& t)
{
	if (ar.exists())
	{
		_bstr_t bstr = ar.value();
		size_t len = sizeof(CharType) == sizeof(char) ? ZERO_TERM : bstr.length();
		iarchive_pchar<CharType>(bstr, len) >> t;
	}
}

template <typename CharType, typename _Traits, typename _Ax>
inline void serialize_save(oarchive_msxml& ar, std::basic_string<CharType, _Traits, _Ax> const& t)
{
	ar.value(t.c_str());
}

template <typename CharType, typename _Traits, typename _Ax>
inline void serialize_save(oarchive_msxml_attr& ar, std::basic_string<CharType, _Traits, _Ax> const& t)
{
	ar.value(t.c_str());
}

//////////////////////////////////////////////////////////////////////////
// 指针类型的序列化，支持多态的方式使用
template <typename T>
struct serialize_loadptr_executor<iarchive_msxml_attr, T>
{
	template <typename Base>
	inline static bool exec(iarchive_msxml_attr const& ar, Base*& p, void*)
	{
		if (!p)
			p = ptr_creator<T>::create();
		ar >> *p;
		return true;
	}
	template <typename Base, typename CharType>
	inline static bool exec(iarchive_msxml_attr const& ar, Base*& p, CharType const* clsname)
	{
		return true;
	}
};

template <typename T>
struct serialize_loadptr_executor<iarchive_msxml, T>
{
	template <typename Base>
	inline static bool exec(iarchive_msxml const& ar, Base*& p, void*)
	{
		if (!p)
			p = ptr_creator<T>::create();
		ar >> *p;
		return true;
	}

	template <typename Base, typename CharType>
	inline static bool exec(iarchive_msxml const& ar, Base*& p, CharType const* clsname)
	{
		bool matched = false;
		if (addon::xml_inherited_tag<Base>::type() == addon::xitt_self)
			matched = ar.equal_name(clsname);
		else if (addon::xml_inherited_tag<Base>::type() == addon::xitt_attr)
		{
			iarchive_msxml_attr attr = ar.attr(addon::xml_inherited_tag<Base>::name((CharType)0));
			matched = attr.exists() && attr.equal_value(clsname);
		}
		else
		{
			iarchive_msxml ch = ar.first_child(addon::xml_inherited_tag<Base>::name((CharType)0));
			matched = ch.exists() && ch.equal_value(clsname);
		}
		if (matched)
		{
			if (!p || typeid(*p) != typeid(T))
				p = ptr_creator<T>::create();
			ar >> *static_cast<T*>(p);
			return true;
		}
		return false;
	}
};

template <typename T>
struct serialize_saveptr_executor<oarchive_msxml, T>
{
	template <typename Base>
	inline static bool exec(oarchive_msxml& ar, Base*const& p, void*)
	{
		if (p)
			ar << *p;
		return true;
	}
	template <typename Base, typename CharType>
	inline static bool exec(oarchive_msxml& ar, Base*const& p, CharType const* clsname)
	{
		if (p && typeid(*p) == typeid(T)) // 使用type_info比较和static_cast性能上应该比dynamic_cast好，并且有静态类型检查
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
					ar.attr(addon::xml_inherited_tag<Base>::name((CharType)0)) << std::basic_string<CharType>(clsname);
				}
				else
				{
					ar << *pt;
					ar.first_child(addon::xml_inherited_tag<Base>::name((CharType)0), NODE_TEXT) << std::basic_string<CharType>(clsname);
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

#define msxml_TRANSLATE_BASIC_TYPE(type) \
inline void serialize_load(iarchive_msxml const& ar, type& t)\
{\
	if (ar.exists())\
	{\
		_bstr_t bstr = ar.value();\
		size_t len = bstr.length();\
		iarchive_pchar<wchar_t>(bstr, len) >> t;\
	}\
}\
inline void serialize_load(iarchive_msxml_attr const& ar, type& t)\
{\
	if (ar.exists())\
	{\
		_bstr_t bstr = ar.value();\
		size_t len = bstr.length();\
		iarchive_pchar<wchar_t>(bstr, len) >> t;\
	}\
}\
inline void serialize_save(oarchive_msxml& ar, type const& t)\
{\
	oarchive_pchar<wchar_t> opc;\
	opc << t;\
	ar.value(opc.pchar);\
}\
inline void serialize_save(oarchive_msxml_attr& ar, type const& t)\
{\
	oarchive_pchar<wchar_t> opc;\
	opc << t;\
	ar.value(opc.pchar);\
}\


msxml_TRANSLATE_BASIC_TYPE(char)
msxml_TRANSLATE_BASIC_TYPE(unsigned char)
msxml_TRANSLATE_BASIC_TYPE(short)
msxml_TRANSLATE_BASIC_TYPE(unsigned short)
msxml_TRANSLATE_BASIC_TYPE(long)
msxml_TRANSLATE_BASIC_TYPE(unsigned long)
msxml_TRANSLATE_BASIC_TYPE(long long)
msxml_TRANSLATE_BASIC_TYPE(unsigned long long)
msxml_TRANSLATE_BASIC_TYPE(int)
msxml_TRANSLATE_BASIC_TYPE(unsigned int)

msxml_TRANSLATE_BASIC_TYPE(float)
msxml_TRANSLATE_BASIC_TYPE(double)
msxml_TRANSLATE_BASIC_TYPE(long double)

msxml_TRANSLATE_BASIC_TYPE(bool)
msxml_TRANSLATE_BASIC_TYPE(void*)


#undef msxml_TRANSLATE_BASIC_TYPE


NS_SERIALIZATION_LITE_END

#endif//__ARCHIVE_MSXML_BASIC_H_AD81B9F0_043C_46DE_861A_A5C30F60D657__

