
#ifndef SRLZ_ARCHIVE_RAPIDXML_H
#define SRLZ_ARCHIVE_RAPIDXML_H

#include "base/srlz/srlz.h"
#include "base/srlz/template_util.h"
#include "base/srlz/srlz_string_util.h"

#include "base/rapidxml/rapidxml.hpp"
#include "base/rapidxml/rapidxml_print.hpp"

#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <list>
#include <deque>
#include <set>
#include <utility>
#include <type_traits>

NS_SERIALIZATION_LITE_BEGIN

//////////////////////////////////////////////////////////////////////////

namespace xml_internal {

// support for writeable buffer of xml source
template<typename CharType>
struct WriteableBuffer 
{
	WriteableBuffer( std::basic_string<CharType> const& src )
		: len_( src.size() )
	{
		array_ = new CharType[len_ + 1];
    memset( array_, 0, (len_+1) * sizeof(CharType) );
		memcpy( array_, src.c_str(), len_ * sizeof(CharType) );
		array_[len_] = 0;
	}

	size_t size() { return len_; }

	CharType* get() { return array_; }

private:
	CharType* array_;
	size_t len_;
};

// typedefs depends on CharType
template<typename CharType>
struct context;

template<>
struct context<char>
{
	static const char Space = ' ';

	static const char* RootName() { return "xparam"; }
	static const char* KeyName() { return "id"; }
	static const char* XmlFilePi() { return "xml version='1.0' encoding='utf-8'"; }
	static std::string ToUtf8( std::string const& str ) { return str; }
	static std::string FromUtf8( std::string const& str ) { return str; }
};

template<>
struct context<wchar_t>
{
	static const wchar_t Space = L' ';

	static const wchar_t* RootName() { return L"xparam"; }
	static const wchar_t* KeyName() { return L"id"; }
	static const wchar_t* XmlFilePi() { return L"xml version='1.0' encoding='utf-8'"; }
	static std::string ToUtf8( std::wstring const& str ) { return base::WideToUTF8(str); }
	static std::wstring FromUtf8( std::string const& str ) { return base::UTF8ToWide(str); }
};

}// xml_internal


////////////////////////////////////////////////////////////////////////
// rapidxml full support utf-8 only, so iarchive_rapidxml do not support for wchat_t later
template <class CharType>
class iarchive_rapidxml;

template <class CharType>
class oarchive_rapidxml;


// class iarchive_rapidxml
template <>
class iarchive_rapidxml<char> : public iarchive_base< iarchive_rapidxml<char> >
{
	typedef rapidxml::xml_node<char> XmlValue;
public:
	iarchive_rapidxml(XmlValue const& val)
		: xmlVal_(val)
	{
	}

	XmlValue const& value() const
	{
		return xmlVal_;
	}
private:
	XmlValue const& xmlVal_;
};

// class oarchive_rapidxml
template <>
class oarchive_rapidxml<char> : public oarchive_base< oarchive_rapidxml<char> >
{
	typedef rapidxml::xml_document<char> XmlDoc;
	typedef rapidxml::xml_node<char> XmlValue;
public:
	oarchive_rapidxml( XmlValue& val, XmlDoc& doc ) 
		: xmlVal_(val), xmlDoc_(doc)
	{}
	
	XmlValue& value()
	{
		return xmlVal_;
	}

	XmlDoc& doc()
	{
		return xmlDoc_;
	}

private:
	XmlValue& xmlVal_;
	XmlDoc& xmlDoc_;
};

//////////////////////////////////////////////////////////////////////////
// for member_nvio

template <typename OwnerType, size_t offset, typename CharType, typename ValueType, typename InitType>
inline void serialize_load(const srlz::iarchive_rapidxml<char>& ar, srlz::member_nvio<OwnerType, offset, CharType, ValueType, InitType>& nvio)
{
	if ( nvio.name == 0 )
	{
		assert(false);
	}
	else
	{
		rapidxml::xml_node<char> const& doc = ar.value();
		std::string utf8Str = xml_internal::context<CharType>::ToUtf8(nvio.name);
		rapidxml::xml_node<char>* node = doc.first_node( utf8Str.c_str() );
		if ( node )
		{
			srlz::iarchive_rapidxml<char>( *node ) >> nvio.value;
		}
	}
}

template <typename OwnerType, size_t offset, typename CharType, typename ValueType, typename InitType>
inline void serialize_save( srlz::oarchive_rapidxml<char>& ar, srlz::member_nvio<OwnerType, offset, CharType, ValueType, InitType> const& nvio)
{
	if (nvio.name == 0)
		assert(false);
	else
	{
		rapidxml::xml_node<char> & root = ar.value();

		std::string utf8Str = xml_internal::context<CharType>::ToUtf8(nvio.name);
		rapidxml::xml_node<char> * node = ar.doc().allocate_node( rapidxml::node_element, ar.doc().allocate_string( utf8Str.c_str() ) );
		root.append_node(node);

		srlz::oarchive_rapidxml<char>( *node, ar.doc() ) << nvio.value;
	}
}

//////////////////////////////////////////////////////////////////////////
// for STL vector or map

namespace detail_xml {

// serialize_save_helper
struct serialize_save_helper
{
	template<typename Container>
	static void serialize_save(oarchive_rapidxml<char>& ar, Container const& t)
	{
		rapidxml::xml_node<char> * parent_node = ar.value().parent();
		std::basic_string<char> name = ar.value().name();
		
		// vector appear in father level
		parent_node->remove_node( &ar.value() );

		for ( typename Container::const_iterator cit = t.begin(); cit != t.end(); ++cit )
		{
			rapidxml::xml_node<char> * node = ar.doc().allocate_node( rapidxml::node_element, ar.doc().allocate_string(name.c_str()) );
			parent_node->append_node(node);

			srlz::oarchive_rapidxml<char>( *node, ar.doc() ) << (*cit);
		}
	}
};

// serialize_load_helper
enum ArrayInsert { INSERT, PUSH_BACK };
/*
insert:		set, multi_set
push_back:	vector, list, deque
*/
template<int I>
struct serialize_load_helper;

template<>
struct serialize_load_helper<INSERT>
{
	template<typename Container>
	static void serialize_load(const iarchive_rapidxml<char>& ar, Container& t)
	{
		std::basic_string<char> name = ar.value().name();

		const rapidxml::xml_node<char> * node = &ar.value();
		while( node )
		{
			srlz::iarchive_rapidxml<char> is(*node);
			typename Container::value_type tv;
			is & tv;
			t.insert( tv );

			node = node->next_sibling( name.c_str() );
		}
	}
};

template<>
struct serialize_load_helper<PUSH_BACK>
{
	template<typename Container>
	static void serialize_load(const iarchive_rapidxml<char>& ar, Container& t)
	{
		std::basic_string<char> name = ar.value().name();

		const rapidxml::xml_node<char> * node = &ar.value();
		while( node )
		{
			srlz::iarchive_rapidxml<char> is(*node);
			typename Container::value_type tv;
			is & tv;
			t.push_back( tv );

			node = node->next_sibling( name.c_str() );
		}
	}
};


}// detail_xml


// vector
template <typename T>
inline void serialize_load(const iarchive_rapidxml<char>& ar, std::vector<T>& t)
{
	detail_xml::serialize_load_helper<detail_xml::PUSH_BACK>::template serialize_load( ar, t );
}

template <typename T>
inline void serialize_save(oarchive_rapidxml<char>& ar, std::vector<T> const& t)
{
	detail_xml::serialize_save_helper::serialize_save(ar, t);
}

// list
template <typename T>
inline void serialize_load(const iarchive_rapidxml<char>& ar, std::list<T>& t)
{
	detail_xml::serialize_load_helper<detail_xml::PUSH_BACK>::template serialize_load( ar, t );
}

template <typename T>
inline void serialize_save(oarchive_rapidxml<char>& ar, std::list<T> const& t)
{
	detail_xml::serialize_save_helper::serialize_save(ar, t);
}

// deque
template <typename T>
inline void serialize_load(const iarchive_rapidxml<char>& ar, std::deque<T>& t)
{
	detail_xml::serialize_load_helper<detail_xml::PUSH_BACK>::template serialize_load( ar, t );
}

template <typename T>
inline void serialize_save(oarchive_rapidxml<char>& ar, std::deque<T> const& t)
{
	detail_xml::serialize_save_helper::serialize_save(ar, t);
}

// set
template <typename T>
inline void serialize_load(const iarchive_rapidxml<char>& ar, std::set<T>& t)
{
	detail_xml::serialize_load_helper<detail_xml::INSERT>::template serialize_load( ar, t );
}

template <typename T>
inline void serialize_save(oarchive_rapidxml<char>& ar, std::set<T> const& t)
{
	detail_xml::serialize_save_helper::serialize_save(ar, t);
}

// multiset
template <typename T>
inline void serialize_load(const iarchive_rapidxml<char>& ar, std::multiset<T>& t)
{
	detail_xml::serialize_load_helper<detail_xml::INSERT>::template serialize_load( ar, t );
}

template <typename T>
inline void serialize_save(oarchive_rapidxml<char>& ar, std::multiset<T> const& t)
{
	detail_xml::serialize_save_helper::serialize_save(ar, t);
}

// map
template <typename Key, typename Val>
inline void serialize_load(const iarchive_rapidxml<char>& ar, std::map<Key, Val>& t)
{
	std::basic_string<char> name = ar.value().name();

	const rapidxml::xml_node<char> * node = &ar.value();
	while( node )
	{
		rapidxml::xml_attribute<char> * attr = node->first_attribute( xml_internal::context<char>::KeyName() );
		if ( attr )
		{
      Key k;
      if ( internal::map_key<Key>::Str2Key(attr->value(), k) ) {
        srlz::iarchive_rapidxml<char> is_val( *node );
        is_val & t[ k ];
      }
		}
		
		node = node->next_sibling( name.c_str() );
	}
}

template <typename Key, typename Val>
inline void serialize_save(oarchive_rapidxml<char>& ar, std::map<Key, Val> const& t)
{
	rapidxml::xml_node<char> * parent_node = ar.value().parent();
	std::basic_string<char> name = ar.value().name();

	// map appear in father level
	parent_node->remove_node( &ar.value() );

	for ( typename std::map<Key, Val>::const_iterator cit = t.begin(); cit != t.end(); ++cit )
	{
		try
		{
			// key need satisfy internal::map_key
			std::string strKey = internal::map_key<Key>::Key2Str(cit->first);
			
			rapidxml::xml_node<char> * node = ar.doc().allocate_node( rapidxml::node_element, ar.doc().allocate_string(name.c_str()) );
			parent_node->append_node(node);

			rapidxml::xml_attribute<char> * attr = 
				ar.doc().allocate_attribute( ar.doc().allocate_string( xml_internal::context<char>::KeyName() ), 
				ar.doc().allocate_string(strKey.c_str()) );

			node->append_attribute( attr );
			srlz::oarchive_rapidxml<char>( *node, ar.doc() ) << (cit->second);
		}
		catch(...)
		{}
	}
}

//////////////////////////////////////////////////////////////////////////
// for string
template <class CharType>
inline void serialize_load(iarchive_rapidxml<char> const& ar, std::basic_string<CharType>& t)
{
	rapidxml::xml_node<char> * data = ar.value().first_node();
	if ( data && 
		(data->type() == rapidxml::node_data || data->type() == rapidxml::node_cdata) )
	{
		t = xml_internal::context<CharType>::FromUtf8( data->value() );
	}
}

template <class CharType>
inline void serialize_save(oarchive_rapidxml<char>& ar, std::basic_string<CharType> const& t)
{
	// later work: test t to decide use CDATA
	rapidxml::xml_node<char>* data = ar.doc().allocate_node( rapidxml::node_data );
	std::string utf8Str = xml_internal::context<CharType>::ToUtf8(t);
	data->value( ar.doc().allocate_string( utf8Str.c_str() ) );
	ar.value().append_node( data );
}

#if defined(OS_WIN)

// for CString & _bstr_t
inline void serialize_load(iarchive_rapidxml<char> const& ar, CString& t)
{
  std::string tmp_str;
  serialize_load( ar, tmp_str );
  t = base::UTF8ToWide(tmp_str).c_str();
}

inline void serialize_save(oarchive_rapidxml<char>& ar, CString const& t)
{
  serialize_save( ar, base::WideToUTF8(t.GetString()) );
}

inline void serialize_load(iarchive_rapidxml<char> const& ar, _bstr_t& t)
{
  std::string tmp_str;
  serialize_load( ar, tmp_str );
  t = base::UTF8ToWide(tmp_str).c_str();
}

inline void serialize_save(oarchive_rapidxml<char>& ar, _bstr_t const& t)
{
    _bstr_t tmp_bstr(t);
    if ( tmp_bstr.length() > 0 )
    {
        serialize_save( ar, base::WideToUTF8(tmp_bstr.GetBSTR()) );
    }
}

#endif

//////////////////////////////////////////////////////////////////////////
// for const char* save
template<typename CharType>
inline void serialize_save(oarchive_rapidxml<char>& ar, const CharType* t)
{
	serialize_save( ar, std::basic_string<CharType>(t) );
}


//////////////////////////////////////////////////////////////////////////
// for other basic types

template <typename T>
inline typename internal::enable_if_c< std::is_integral<T>::value >::type 
serialize_load(iarchive_rapidxml<char> const& ar, T& t)
{
	std::basic_string<char> data;
	serialize_load( ar, data );

  int64 val = 0;
  bool suc = base::StringToInt64(data, &val);
  if ( suc ) {
    t = (T)val;
  }
}

template <typename T>
inline typename internal::enable_if_c< std::is_integral<T>::value >::type 
serialize_save(oarchive_rapidxml<char>& ar, const T& t)
{
  std::string data = base::Int64ToString((int64)t);
	serialize_save(ar, data);
}


// for enum
template <typename EnumType>
inline typename internal::enable_if_c< std::is_enum<EnumType>::value >::type
serialize_load(iarchive_rapidxml<char> const& ar, EnumType& t)
{
	int val = 0;
	serialize_load(ar, val);
	t = (EnumType)val;
}

template <typename EnumType>
inline typename internal::enable_if_c< std::is_enum<EnumType>::value >::type
serialize_save(oarchive_rapidxml<char>& ar, const EnumType& t)
{
	serialize_save(ar, (int)t);
}


NS_SERIALIZATION_LITE_END



#endif //SRLZ_ARCHIVE_RAPIDXML_H


