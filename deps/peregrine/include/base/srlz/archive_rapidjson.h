
#ifndef SRLZ_ARCHIVE_RAPIDJSON_H
#define SRLZ_ARCHIVE_RAPIDJSON_H


#include "base/srlz/srlz.h"
#include "base/srlz/template_util.h"
#include "base/srlz/srlz_string_util.h"

#include "base/rapidjson/document.h"
#include "base/rapidjson/prettywriter.h"
#include "base/rapidjson/stringbuffer.h"

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


namespace json_internal {

// just support UTF-8
typedef rapidjson::UTF8<> Encoding;
typedef rapidjson::Value Value;
typedef rapidjson::Document Document;
typedef rapidjson::StringBuffer StringBuffer;

template<typename T, typename U>
struct SetDefault
{
	static void set(T&, U const&) {}
};

template<typename T>
struct SetDefault<T,T>
{
	static void set(T& v, T const& defv) { v = defv; }
};

}// json_internal

// class iarchive_rapidjson
class iarchive_rapidjson : public iarchive_base< iarchive_rapidjson >
{
public:
	typedef rapidjson::Value JsonValue;

	iarchive_rapidjson(JsonValue const& val)
		: jsonVal_(val)
	{
	}

	JsonValue const& value() const
	{
		return jsonVal_;
	}
private:
	JsonValue const& jsonVal_;
};

//////////////////////////////////////////////////////////////////////////
// class oarchive_rapidjson
class oarchive_rapidjson : public oarchive_base< oarchive_rapidjson >
{
public:
	typedef rapidjson::Value JsonValue;

	oarchive_rapidjson( JsonValue& val, rapidjson::MemoryPoolAllocator<>& allocator ) 
		: jsonVal_(val), allocator_(allocator)
	{}
	
	JsonValue& value()
	{
		return jsonVal_;
	}

	rapidjson::MemoryPoolAllocator<>& allocator()
	{
		return allocator_;
	}

private:
	JsonValue& jsonVal_;
	rapidjson::MemoryPoolAllocator<>& allocator_;
};

//////////////////////////////////////////////////////////////////////////
// for member_nvio

template <typename OwnerType, size_t offset, typename CharType, typename ValueType, typename InitType>
inline void serialize_load(const iarchive_rapidjson& ar, member_nvio<OwnerType, offset, CharType, ValueType, InitType>& nvio)
{
	if ( nvio.name == 0 )
	{
		assert(false);
	}
	else
	{
		rapidjson::Value const& doc = ar.value();
		if ( doc.IsObject() )
        {
            std::string key = internal::StringNameConvert(nvio.name);
            if ( doc.HasMember(key.c_str()) )
            {
                rapidjson::Value const& member_val = doc[key.c_str()];
                if ( !member_val.IsNull() )
                {
                    iarchive_rapidjson( member_val ) >> nvio.value;
                    return;
                }
            }

			json_internal::SetDefault<ValueType,InitType>::set( nvio.value, nvio.ival );
		}
	}
}

template <typename OwnerType, size_t offset, typename CharType, typename ValueType, typename InitType>
inline void serialize_save( oarchive_rapidjson& ar, member_nvio<OwnerType, offset, CharType, ValueType, InitType> const& nvio)
{
	if (nvio.name == 0)
		assert(false);
	else
	{
		rapidjson::Value & val = ar.value();
		if ( !val.IsObject() )
		{
			val.SetObject();
		}

        std::string key = internal::StringNameConvert(nvio.name);
		rapidjson::Value obj_key(key.c_str(), ar.allocator());

		rapidjson::Value obj_val;
		oarchive_rapidjson( obj_val, ar.allocator() ) << nvio.value;
		val.AddMember(obj_key, obj_val, ar.allocator());
	}
}

//////////////////////////////////////////////////////////////////////////
// for STL vector or map

namespace json_detail {

// serialize_save_helper
struct serialize_save_helper
{
	template<typename Container>
	static void serialize_save(oarchive_rapidjson& ar, Container const& t)
	{
		rapidjson::Value & val = ar.value();
		if ( !val.IsArray() )
		{
			val.SetArray();
		}

		for ( typename Container::const_iterator cit = t.begin(); cit != t.end(); ++cit )
		{
			rapidjson::Value tmp;
			oarchive_rapidjson os(tmp, ar.allocator());
			os & (*cit);
			val.PushBack( os.value(), ar.allocator() );
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
	static void serialize_load(const iarchive_rapidjson& ar, Container& t)
	{
		rapidjson::Value const& val = ar.value();
		if ( val.IsArray() )
		{
			t.clear();
			
			for ( rapidjson::Value::ConstValueIterator cit = val.Begin(); 
				cit != val.End(); ++cit )
			{
				iarchive_rapidjson is( *cit );
				typename Container::value_type tv;
				is & tv;

				t.insert( tv );
			}
		}
	}
};

template<>
struct serialize_load_helper<PUSH_BACK>
{
	template<typename Container>
	static void serialize_load(const iarchive_rapidjson& ar, Container& t)
	{
		rapidjson::Value const& val = ar.value();
		if ( val.IsArray() )
		{
			t.clear();

			for ( rapidjson::Value::ConstValueIterator cit = val.Begin(); 
				cit != val.End(); ++cit )
			{
				iarchive_rapidjson is( *cit );
				typename Container::value_type tv;
				is & tv;

				t.push_back( tv );
			}
		}
	}
};


}// json_detail


// vector
template <typename T>
inline void serialize_load(const iarchive_rapidjson& ar, std::vector<T>& t)
{
	json_detail::serialize_load_helper<json_detail::PUSH_BACK>::template serialize_load( ar, t );
}

template <typename T>
inline void serialize_save(oarchive_rapidjson& ar, std::vector<T> const& t)
{
	json_detail::serialize_save_helper::serialize_save(ar, t);
}

// list
template <typename T>
inline void serialize_load(const iarchive_rapidjson& ar, std::list<T>& t)
{
	json_detail::serialize_load_helper<json_detail::PUSH_BACK>::template serialize_load( ar, t );
}

template <typename T>
inline void serialize_save(oarchive_rapidjson& ar, std::list<T> const& t)
{
	json_detail::serialize_save_helper::serialize_save(ar, t);
}

// deque
template <typename T>
inline void serialize_load(const iarchive_rapidjson& ar, std::deque<T>& t)
{
	json_detail::serialize_load_helper<json_detail::PUSH_BACK>::template serialize_load( ar, t );
}

template <typename T>
inline void serialize_save(oarchive_rapidjson& ar, std::deque<T> const& t)
{
	json_detail::serialize_save_helper::serialize_save(ar, t);
}

// set
template <typename T>
inline void serialize_load(const iarchive_rapidjson& ar, std::set<T>& t)
{
	json_detail::serialize_load_helper<json_detail::INSERT>::template serialize_load( ar, t );
}

template <typename T>
inline void serialize_save(oarchive_rapidjson& ar, std::set<T> const& t)
{
	json_detail::serialize_save_helper::serialize_save(ar, t);
}

// multiset
template <typename T>
inline void serialize_load(const iarchive_rapidjson& ar, std::multiset<T>& t)
{
	json_detail::serialize_load_helper<json_detail::INSERT>::template serialize_load( ar, t );
}

template <typename T>
inline void serialize_save(oarchive_rapidjson& ar, std::multiset<T> const& t)
{
	json_detail::serialize_save_helper::serialize_save(ar, t);
}

// map
template <typename Key, typename Val>
inline void serialize_load(const iarchive_rapidjson& ar, std::map<Key, Val>& t)
{
	rapidjson::Value const& val = ar.value();
	if ( val.IsObject() )
	{
		t.clear();

		for ( rapidjson::Value::ConstMemberIterator cit = val.MemberBegin(); 
			cit != val.MemberEnd(); ++cit )
		{
            Key k;
            if ( internal::map_key<Key>::Str2Key(cit->name.GetString(), k) ) {
                iarchive_rapidjson is_val( cit->value );
                is_val & t[ k ];
            }
		}
	}
}

template <typename Key, typename Val>
inline void serialize_save(oarchive_rapidjson& ar, std::map<Key, Val> const& t)
{
	rapidjson::Value & val = ar.value();
	if ( !val.IsObject() )
	{
		val.SetObject();
	}

	for ( typename std::map<Key, Val>::const_iterator cit = t.begin(); cit != t.end(); ++cit )
	{
		try 
		{
			std::string strKey = internal::map_key<Key>::Key2Str( cit->first );
			rapidjson::Value tmp_val;
			oarchive_rapidjson os_val(tmp_val, ar.allocator());
			os_val & (cit->second);

			// string need to be stored internal
			rapidjson::Value member_key( strKey.c_str(), (unsigned)strKey.size(), ar.allocator() );
			val.AddMember( member_key, os_val.value(), ar.allocator() );
		}
		catch (...)
		{
		}
	}
}

/*
Note: multimap与json的Object结构不兼容
*/


//////////////////////////////////////////////////////////////////////////
// for string
template <class CharType>
inline void serialize_load(iarchive_rapidjson const& ar, std::basic_string<CharType>& t)
{
	if ( ar.value().IsString() )
	{
        t = internal::StringConvert<CharType>::invoke( ar.value().GetString() );
	}
	else
		assert(false);
}

template <class CharType>
inline void serialize_save(oarchive_rapidjson& ar, std::basic_string<CharType> const& t)
{
    std::string tmp = internal::StringNameConvert(t.c_str());
	ar.value().SetString( tmp.c_str(), (unsigned)tmp.size(), ar.allocator() );
}


#if defined(OS_WIN)

// for CString & _bstr_t
inline void serialize_load(iarchive_rapidjson const& ar, CString& t)
{
  std::string tmp_str;
  serialize_load( ar, tmp_str );
  t = base::UTF8ToWide(tmp_str).c_str();
}

inline void serialize_save(oarchive_rapidjson& ar, CString const& t)
{
  serialize_save( ar, base::WideToUTF8(t.GetString()) );
}

inline void serialize_load(iarchive_rapidjson const& ar, _bstr_t& t)
{
  std::string tmp_str;
  serialize_load( ar, tmp_str );
  t = base::UTF8ToWide(tmp_str).c_str();
}

inline void serialize_save(oarchive_rapidjson& ar, _bstr_t const& t)
{
    _bstr_t tmp_bstr(t);
    if ( tmp_bstr.length() > 0 )
    {
        serialize_save( ar, base::WideToUTF8(tmp_bstr.GetBSTR()) );
    }
}

#endif
//////////////////////////////////////////////////////////////////////////
// for other basic types
#define STRING_TRANSLATE_INTEGER_TYPE(name,type) \
inline void serialize_load(iarchive_rapidjson const& ar, type& t)\
{\
	rapidjson::Value const& val = ar.value();\
	if ( val.Is##name() )\
	{\
		t = val.Get##name();\
	}\
}\
inline void serialize_save(oarchive_rapidjson& ar, const type& t)\
{\
	ar.value() = t;\
}

STRING_TRANSLATE_INTEGER_TYPE(Int,int)
STRING_TRANSLATE_INTEGER_TYPE(Uint,unsigned)
STRING_TRANSLATE_INTEGER_TYPE(Int64,int64_t)
STRING_TRANSLATE_INTEGER_TYPE(Uint64,uint64_t)
STRING_TRANSLATE_INTEGER_TYPE(Double,double)
STRING_TRANSLATE_INTEGER_TYPE(Bool,bool)

#undef STRING_TRANSLATE_INTEGER_TYPE


inline void serialize_load(iarchive_rapidjson const& ar, long& t)
{
	if ( ar.value().IsInt() )
	{
		t = (long)ar.value().GetInt();
	}
}

inline void serialize_save(oarchive_rapidjson& ar, const long& t)
{
	ar.value() = (int)t;
}


inline void serialize_load(iarchive_rapidjson const& ar, unsigned long& t)
{
	if ( ar.value().IsInt() )
	{
		t = (unsigned long)ar.value().GetInt();
	}
}

inline void serialize_save(oarchive_rapidjson& ar, const unsigned long& t)
{
	ar.value() = (int)t;
}

inline void serialize_load(iarchive_rapidjson const& ar, unsigned short& t)
{
	if ( ar.value().IsInt() )
	{
		t = (unsigned short)ar.value().GetInt();
	}
}

inline void serialize_save(oarchive_rapidjson& ar, const unsigned short& t)
{
	ar.value() = (unsigned short)t;
}

// for enum
template <typename EnumType>
inline typename internal::enable_if_c< std::is_enum<EnumType>::value >::type
serialize_load(iarchive_rapidjson const& ar, EnumType& t)
{
	if ( ar.value().IsInt() )
	{
		t = (EnumType)ar.value().GetInt();
	}
}

template <typename EnumType>
inline typename internal::enable_if_c< std::is_enum<EnumType>::value >::type
serialize_save(oarchive_rapidjson& ar, const EnumType& t)
{
	ar.value() = (int)t;
}


// rapidjson::Document: has separate allocator
inline void serialize_load(iarchive_rapidjson const& ar, rapidjson::Document& t)
{
    if ( ar.value().IsArray() || ar.value().IsObject() )
    {
        rapidjson::StringBuffer strbuf;
        rapidjson::Writer< rapidjson::StringBuffer, rapidjson::UTF8<> > writer(strbuf);
        ar.value().Accept( writer );
        
        t.Parse( strbuf.GetString() );
    }
}

inline void serialize_save(oarchive_rapidjson& ar, const rapidjson::Document& t)
{
    ar.value().CopyFrom(t, ar.allocator());
}

NS_SERIALIZATION_LITE_END



#endif //SRLZ_ARCHIVE_RAPIDJSON_H


