#pragma once

#include "base/srlz/archive_rapidxml.h"

namespace rapid_xml { 

// 从结构对象 转为 xml字符串
// struct直接支持，其他如map, vector,int,string等需要使用SL_SINGLE_ITEM
template<typename CharType, typename T>
inline bool XmlStrFormClass( std::basic_string<CharType> & xml_str, T const& val, 
	std::basic_string<char> rootName = srlz::xml_internal::context<char>::RootName() , 
	bool file_pi = false )
{
	std::string utf8Str;
	rapidxml::xml_document<char> doc;
	if ( file_pi )
	{
		rapidxml::xml_node<char> * pi = 
			doc.allocate_node(rapidxml::node_pi, doc.allocate_string(srlz::xml_internal::context<char>::XmlFilePi()));  
		doc.append_node(pi);
	}

	rapidxml::xml_node<char> * root = 
		doc.allocate_node(rapidxml::node_element, doc.allocate_string(rootName.c_str()));
	doc.append_node(root);

	srlz::oarchive_rapidxml<char> os( *root, doc );
	os & val;

	rapidxml::print( std::back_inserter(utf8Str), doc, 0);
	xml_str = srlz::xml_internal::context<CharType>::FromUtf8( utf8Str );
	return true;
}

// 从xml字符串 转为 结构对象
// struct直接支持，其他如map, vector,int,string等需要使用SL_SINGLE_ITEM
template<typename CharType, typename T>
inline bool XmlStrToClass( std::basic_string<CharType> const& xml_str, T & val )
{
    try
    {
        std::string utf8Str = srlz::xml_internal::context<CharType>::ToUtf8( xml_str );
        rapidxml::xml_document<char> doc;
        srlz::xml_internal::WriteableBuffer<char> wb( utf8Str );
        doc.parse<0>(wb.get());

        rapidxml::xml_node<char> * root = doc.first_node();
        if ( root )
        {
            srlz::iarchive_rapidxml<char> is( *root );
            is & val;
            return true;
        }
    }
    catch (...)
    {
    	
    }

	return false;
}

}// rapid_xml
