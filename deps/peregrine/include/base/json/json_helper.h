#pragma once

#include "base/srlz/archive_json.h"

#include "base/file_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/strings/gbk_string_conversions.h"

namespace json {

// 从结构对象 转为 json对象
template<typename T>
inline scoped_ptr<base::Value> JsonFromClass( T const& val )
{
	scoped_ptr<base::Value> result;
	srlz::oarchive_json os( result );
	os & val;

	return result.Pass();
}

template<typename T>
inline void JsonFromClass( scoped_ptr<base::Value>& obj, T const& val )
{
	srlz::oarchive_json os( obj );
	os & val;
}

// 从json对象 转为 结构对象
template<typename T>
inline void JsonToClass( scoped_ptr<base::Value> const& obj, T & val )
{
	srlz::iarchive_json is( obj.get() );
	is & val;
}

// 从结构对象 转为 json字符串
template<typename T>
inline std::string JsonStrFromClass( T const& val, bool formated = false )
{
  scoped_ptr<base::Value> result;
  srlz::oarchive_json os( result );
  os & val;
  
  std::string json;

  if (result->GetType() == base::Value::TYPE_STRING) {
    result->GetAsString(&json);
    return json;
  }
  
	if ( formated )	{
    base::JSONWriter::WriteWithOptions( result.get(),
                                       base::JSONWriter::OPTIONS_DO_NOT_ESCAPE | base::JSONWriter::OPTIONS_PRETTY_PRINT, &json );
	}
  else {
    base::JSONWriter::WriteWithOptions( result.get(), base::JSONWriter::OPTIONS_DO_NOT_ESCAPE, &json );
  }
		
  return json;
}

// 从json字符串 转为 结构对象
template<typename T>
inline bool JsonStrToClass( std::string const& json_str, T & val )
{
  scoped_ptr<base::Value> obj( base::JSONReader::Read(json_str) );
  if ( obj ) {
    JsonToClass( obj, val );
    return true;
  }

  return false;
}

template<typename T>
inline std::wstring JsonWStrFromClass( T const& val, bool formated = false )
{
  std::string utf8Str = JsonStrFromClass(val, formated);
  return base::UTF8ToWide(utf8Str);
}

template<typename T>
inline bool JsonStrToClass( std::wstring const& wide_json_str, T & val )
{
    std::string json_str = base::WideToUTF8(wide_json_str);
    return JsonStrToClass(json_str, val);
}


// 以下两个函数涉及文件同步操作，UI上不要用
	
// 从结构对象 转为 json文件
template<typename T>
inline bool JsonFileFromClass( base::FilePath file_path, T const& val, bool formated = true )
{
	std::string fileData = JsonStrFromClass(val, formated);
	return (file_util::WriteFile(file_path, fileData.c_str(), (int)fileData.size()) != -1);
}

// 从json字符串 转为 结构对象
template<typename T>
inline bool JsonFileToClass( base::FilePath file_path, T & val )
{
	std::string fileData;
	bool success = base::ReadFileToString( file_path, &fileData );
	if ( success ) {
		return JsonStrToClass(fileData, val);
	}
	
	return false;
}

}// json
