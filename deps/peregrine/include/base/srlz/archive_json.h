#pragma once

#include "base/srlz/srlz.h"
#include "base/srlz/srlz_string_util.h"
#include "base/srlz/template_util.h"

#include "base/values.h"
#include "base/json/json_writer.h"
#include "base/json/json_reader.h"

#include "base/memory/scoped_ptr.h"
#include "base/memory/linked_ptr.h"

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
// class iarchive_json
class iarchive_json : public iarchive_base< iarchive_json >
{
public:
	iarchive_json(const base::Value* val)
		: jsonVal_(val)
	{
	}

	const base::Value* value() const
	{
		return jsonVal_;
	}
private:
  // 基于性能考虑使用const指针，避免中间计算的copy
  const base::Value* jsonVal_;
};

//////////////////////////////////////////////////////////////////////////
// class oarchive_json
class oarchive_json : public oarchive_base< oarchive_json >
{
public:
  oarchive_json( scoped_ptr<base::Value> & val )
		: jsonVal_(val) 
	{}
	
	scoped_ptr<base::Value> & value()
	{
		return jsonVal_;
	}
private:
	scoped_ptr<base::Value> & jsonVal_;
};


//////////////////////////////////////////////////////////////////////////

// for member_nvio

template <typename OwnerType, size_t offset, typename CharType, typename ValueType, typename InitType>
inline void serialize_load(const iarchive_json& ar, member_nvio<OwnerType, offset, CharType, ValueType, InitType>& nvio)
{
  const base::Value* root = ar.value();
  if ( root && root->GetType() == base::Value::TYPE_DICTIONARY ) {
    const base::DictionaryValue * dict = nullptr;
    root->GetAsDictionary(&dict);
    if ( dict ) {
        std::string key = srlz::internal::StringNameConvert(nvio.name);
      const base::Value * val = nullptr;
      dict->Get( key, &val );
      if ( val ) {
        iarchive_json( val ) >> nvio.value;
      }
    }
  }
}

template <typename OwnerType, size_t offset, typename CharType, typename ValueType, typename InitType>
inline void serialize_save( oarchive_json& ar, member_nvio<OwnerType, offset, CharType, ValueType, InitType> const& nvio)
{
  scoped_ptr<base::Value> & root = ar.value();
  if ( !root.get() || root->GetType() != base::Value::TYPE_DICTIONARY ) {
    root = make_scoped_ptr(new base::DictionaryValue());
  }

  base::DictionaryValue * dict = nullptr;
  root->GetAsDictionary(&dict);
  
  scoped_ptr<base::Value> val;
  oarchive_json(val) << nvio.value;
  if ( val ) {
    std::string key = srlz::internal::StringNameConvert(nvio.name);
    dict->Set( key, val.release() );
  }
}

//////////////////////////////////////////////////////////////////////////
// for STL vector or map

namespace detail {

// serialize_save_helper
struct serialize_save_helper
{
	template<typename Container>
	static void serialize_save(oarchive_json& ar, Container const& t)
	{
    scoped_ptr<base::Value> & value = ar.value();
    if ( !value.get() ) {
      value = make_scoped_ptr(new base::ListValue());
    }
      
		if ( value->GetType() != base::Value::TYPE_LIST )
		{
      return;
		}
    
    base::ListValue * ary = nullptr;
    value->GetAsList(&ary);

		for ( typename Container::const_iterator cit = t.begin(); cit != t.end(); ++cit )
		{
      scoped_ptr<base::Value> tmpVal;
			oarchive_json os(tmpVal);
			os & (*cit);
      
      if (tmpVal.get()) {
        ary->Append( tmpVal.release() );
      }
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
	static void serialize_load(const iarchive_json& ar, Container& t)
	{
    const base::Value * value = ar.value();
		if ( value && value->GetType() == base::Value::TYPE_LIST )
		{
      const base::ListValue * ary = nullptr;
      value->GetAsList(&ary);
      if ( ary ) {
        t.clear();
        
        for (auto it = ary->begin(); it != ary->end(); ++it) {
          iarchive_json is( *it );
          
          typename Container::value_type tv;
          is & tv;
          t.insert( tv );
        }
      }
		}
	}
};

template<>
struct serialize_load_helper<PUSH_BACK>
{
	template<typename Container>
	static void serialize_load(const iarchive_json& ar, Container& t)
	{
    const base::Value * value = ar.value();
    if ( value && value->GetType() == base::Value::TYPE_LIST )
    {
      const base::ListValue * ary = nullptr;
      value->GetAsList(&ary);
      if ( ary ) {
        t.clear();
        
        for (auto it = ary->begin(); it != ary->end(); ++it) {
          iarchive_json is( *it );
          
          typename Container::value_type tv;
          is & tv;
          t.push_back( tv );
        }
      }
    }
	}
};


}// detail


// vector
template <typename T>
inline void serialize_load(const iarchive_json& ar, std::vector<T>& t)
{
	detail::serialize_load_helper<detail::PUSH_BACK>::template serialize_load( ar, t );
}

template <typename T>
inline void serialize_save(oarchive_json& ar, std::vector<T> const& t)
{
	detail::serialize_save_helper::serialize_save(ar, t);
}

// list
template <typename T>
inline void serialize_load(const iarchive_json& ar, std::list<T>& t)
{
	detail::serialize_load_helper<detail::PUSH_BACK>::template serialize_load( ar, t );
}

template <typename T>
inline void serialize_save(oarchive_json& ar, std::list<T> const& t)
{
	detail::serialize_save_helper::serialize_save(ar, t);
}

// deque
template <typename T>
inline void serialize_load(const iarchive_json& ar, std::deque<T>& t)
{
	detail::serialize_load_helper<detail::PUSH_BACK>::template serialize_load( ar, t );
}

template <typename T>
inline void serialize_save(oarchive_json& ar, std::deque<T> const& t)
{
	detail::serialize_save_helper::serialize_save(ar, t);
}

// set
template <typename T>
inline void serialize_load(const iarchive_json& ar, std::set<T>& t)
{
	detail::serialize_load_helper<detail::INSERT>::template serialize_load( ar, t );
}

template <typename T>
inline void serialize_save(oarchive_json& ar, std::set<T> const& t)
{
	detail::serialize_save_helper::serialize_save(ar, t);
}

// multiset
template <typename T>
inline void serialize_load(const iarchive_json& ar, std::multiset<T>& t)
{
	detail::serialize_load_helper<detail::INSERT>::template serialize_load( ar, t );
}

template <typename T>
inline void serialize_save(oarchive_json& ar, std::multiset<T> const& t)
{
	detail::serialize_save_helper::serialize_save(ar, t);
}

template <typename Key, typename Val>
inline void serialize_load(const iarchive_json& ar, std::map<Key, Val>& t)
{
  const base::Value* val = ar.value();
  if ( val&& val->GetType() == base::Value::TYPE_DICTIONARY ) {
    const base::DictionaryValue * dict = nullptr;
    val->GetAsDictionary(&dict);
    if ( dict ) {
      t.clear();
      
      for (base::DictionaryValue::Iterator it(*dict); !it.IsAtEnd(); it.Advance()) {
        Key k;
          if ( srlz::internal::map_key<Key>::Str2Key(it.key(), k) ) {
          iarchive_json is_val(&it.value());
          is_val & t[ k ];
        }
      }
    }
  }
}

template <typename Key, typename Val>
inline void serialize_save(oarchive_json& ar, std::map<Key, Val> const& t)
{
	scoped_ptr<base::Value> & val = ar.value();
  if ( !val.get() || val->GetType() != base::Value::TYPE_DICTIONARY ) {
    val = make_scoped_ptr( new base::DictionaryValue() );
  }
  
  base::DictionaryValue * dict = nullptr;
  val->GetAsDictionary(&dict);
  
	for ( typename std::map<Key, Val>::const_iterator cit = t.begin(); cit != t.end(); ++cit )
	{
    scoped_ptr<base::Value> tmp_val;
    oarchive_json(tmp_val) & cit->second;
    if ( tmp_val ) {
      std::string key = srlz::internal::map_key<Key>::Key2Str(cit->first);
      
      dict->Set( key, tmp_val.release() );
    }
	}
}

/*
Note: multimap与json的Object结构不兼容
*/


//////////////////////////////////////////////////////////////////////////
// for string
inline void serialize_load(iarchive_json const& ar, std::string& t)
{
  const base::Value* value = ar.value();
  if ( value && value->GetType() == base::Value::TYPE_STRING ) {
    value->GetAsString(&t);
  }
}

inline void serialize_save(oarchive_json& ar, std::string const& t)
{
  ar.value() = make_scoped_ptr( new base::StringValue(t) );
}

inline void serialize_load(iarchive_json const& ar, std::wstring& t)
{
  std::string tmp_str;
  serialize_load( ar, tmp_str );
  t = base::UTF8ToWide(tmp_str);
}

inline void serialize_save(oarchive_json& ar, std::wstring const& t)
{
  serialize_save( ar, base::WideToUTF8(t) );
}

#if defined(OS_WIN)

// for CString & _bstr_t
inline void serialize_load(iarchive_json const& ar, CString& t)
{
	std::string tmp_str;
	serialize_load( ar, tmp_str );
	t = base::UTF8ToWide(tmp_str).c_str();
}

inline void serialize_save(oarchive_json& ar, CString const& t)
{
	serialize_save( ar, base::WideToUTF8(t.GetString()) );
}

inline void serialize_load(iarchive_json const& ar, _bstr_t& t)
{
	std::string tmp_str;
	serialize_load( ar, tmp_str );
	t = base::UTF8ToWide(tmp_str).c_str();
}

inline void serialize_save(oarchive_json& ar, _bstr_t const& t)
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


inline void serialize_load(iarchive_json const& ar, int& t)
{
  const base::Value* value = ar.value();
  if ( value && value->GetType() == base::Value::TYPE_INTEGER ) {
    value->GetAsInteger(&t);
  }
}

inline void serialize_save(oarchive_json& ar, int const& t)
{
  ar.value() = make_scoped_ptr( new base::FundamentalValue(t) );
}

inline void serialize_load(iarchive_json const& ar, int64& t)
{
  const base::Value* value = ar.value();
  if ( value && value->GetType() == base::Value::TYPE_INTEGER ) {
    value->GetAsInteger(&t);
  }
}

inline void serialize_save(oarchive_json& ar, int64 const& t)
{
  ar.value() = make_scoped_ptr( new base::FundamentalValue(t) );
}

inline void serialize_load(iarchive_json const& ar, double& t)
{
  const base::Value* value = ar.value();
  if ( value && value->GetType() == base::Value::TYPE_DOUBLE ) {
    value->GetAsDouble(&t);
  }
}

inline void serialize_save(oarchive_json& ar, double const& t)
{
  ar.value() = make_scoped_ptr( new base::FundamentalValue(t) );
}

inline void serialize_load(iarchive_json const& ar, bool& t)
{
  const base::Value* value = ar.value();
  if ( value && value->GetType() == base::Value::TYPE_BOOLEAN ) {
    value->GetAsBoolean(&t);
  }
}

inline void serialize_save(oarchive_json& ar, bool const& t)
{
  ar.value() = make_scoped_ptr( new base::FundamentalValue(t) );
}

//////////////////////////////////////////////////////////////////////////
// for other integral types with int & int64
template <typename IntegralType>
inline typename internal::enable_if_c< std::is_integral<IntegralType>::value >::type
serialize_load(iarchive_json const& ar, IntegralType& t)
{
	typedef typename internal::if_< (sizeof(IntegralType)<=sizeof(int)), int, int64 >::type IntType;
  
  IntType val = 0;
  serialize_load( ar, val );
  t = (IntegralType)val;
}

template <typename IntegralType>
inline typename internal::enable_if_c< std::is_integral<IntegralType>::value >::type
serialize_save(oarchive_json& ar, const IntegralType& t)
{
	typedef typename internal::if_< (sizeof(IntegralType)<=sizeof(int)), int, int64 >::type IntType;
  
  serialize_save( ar, (IntType)t );
}

// for enum
template <typename EnumType>
inline typename internal::enable_if_c< std::is_enum<EnumType>::value >::type
serialize_load(iarchive_json const& ar, EnumType& t)
{
  int val = 0;
  serialize_load( ar, val );
  t = (EnumType)val;
}

template <typename EnumType>
inline typename internal::enable_if_c< std::is_enum<EnumType>::value >::type
serialize_save(oarchive_json& ar, const EnumType& t)
{
  serialize_save( ar, (int)t );
}

// scoped_ptr<base::Value>
inline void serialize_load(iarchive_json const& ar, scoped_ptr<base::Value>& t)
{
  if ( ar.value() ) {
    t = make_scoped_ptr( ar.value()->DeepCopy() );
  }
}

inline void serialize_save(oarchive_json& ar, scoped_ptr<base::Value> const& t)
{
  if ( t ) {
    ar.value() = make_scoped_ptr( t->DeepCopy() );
  }
}

// linked_ptr<base::Value>
inline void serialize_load(iarchive_json const& ar, linked_ptr<base::Value>& t)
{
	if ( ar.value() ) {
		t = make_linked_ptr( ar.value()->DeepCopy() );
	}
}

inline void serialize_save(oarchive_json& ar, linked_ptr<base::Value> const& t)
{
	if ( t.get() ) {
		ar.value() = make_scoped_ptr( t->DeepCopy() );
	}
}

NS_SERIALIZATION_LITE_END






