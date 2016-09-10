#pragma once

#include "base/srlz/archive_rapidjson.h"

namespace rapid_json { 

// 从结构对象 转为 json对象
template<typename T>
inline void JsonFromClass( rapidjson::Value & obj, T const& val, rapidjson::MemoryPoolAllocator<>& allocator )
{
	srlz::oarchive_rapidjson os( obj, allocator );
	os & val;
}

// 从json对象 转为 结构对象
template<typename T>
inline void JsonToClass( rapidjson::Value const& obj, T & val )
{
	srlz::iarchive_rapidjson is( obj );
	is & val;
}

// 从结构对象 转为 json字符串
// （struct, map, vector, ie.）等直接支持，其他如int,string等需要使用SL_SINGLE_ITEM
template<typename T>
inline void JsonStrFromClass( std::string & json_str, T const& val, bool formated = false )
{
	rapidjson::MemoryPoolAllocator<> allocator;
	rapidjson::Value result;
	srlz::oarchive_rapidjson os( result, allocator );
	os & val;

	if ( os.value().IsArray() || os.value().IsObject() )
	{
		rapidjson::StringBuffer strbuf;
		if ( formated )
		{
			rapidjson::PrettyWriter< rapidjson::StringBuffer, rapidjson::UTF8<> > writer(strbuf);

			result.Accept( writer );
		}
		else
		{
			rapidjson::Writer< rapidjson::StringBuffer, rapidjson::UTF8<> > writer(strbuf);

			result.Accept( writer );
		}

		json_str = strbuf.GetString();
	}
	else
	{
		assert(false && "val should be object or array, use 'SL_SINGLE_ITEM(val)' instead.");
	}
}

template<typename T>
inline std::string JsonStrFromClass( T const& val, bool formated = false )
{
    std::string json_str;
    JsonStrFromClass(json_str, val, formated);
    return json_str;
}

// for wstring
template<typename T>
inline void JsonStrFromClass( std::wstring & json_str, T const& val, bool formated = false )
{
    std::string tmpStr = JsonStrFromClass(val, formated);
    json_str = base::UTF8ToWide(tmpStr);
}

// 从json字符串 转为 结构对象
// （struct, map, vector, ie.）等直接支持，其他如int,string等需要使用SL_SINGLE_ITEM
template<typename T>
inline bool JsonStrToClass( std::string const& json_str, T & val )
{
	rapidjson::Document doc;
	doc.Parse<0>( json_str.c_str() );
	if ( doc.HasParseError() )
	{
		assert(false && "json_str should be an array or object.");
		return false;
	}

	srlz::iarchive_rapidjson is( doc );
	is & val;
	return true;
}

}// rapid_json
