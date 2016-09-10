#pragma once

#include "base/json/json_helper.h"

#include <vector>
#include <map>
#include <string>
#include <cassert>
#include <sstream>
#include <stdexcept>

namespace json_spirit {
  
  enum Value_type { obj_type, array_type, str_type, bool_type, int_type, real_type, null_type };
  
  template< class tchar >
  class Value_impl
  {
  public:
    typedef std::basic_string<tchar>  String_type;
    typedef base::DictionaryValue     Object;
    typedef base::ListValue           Array;
    typedef typename String_type::const_pointer Const_str_ptr;
    
    Value_impl();
    Value_impl( Const_str_ptr      value );
    Value_impl( const String_type& value );
    Value_impl( const Object*      value );
    Value_impl( const Array*       value );
    Value_impl( bool               value );
    Value_impl( int                value );
    Value_impl( int64              value );
    Value_impl( double             value );
    Value_impl( const Value_impl&  other );
    Value_impl( const base::Value* other );
    
    Value_impl& operator = ( const Value_impl& lhs );
    Value_impl& operator = ( const base::Value* lhs );
    
    bool operator==( const Value_impl& lhs ) const;
    
    Value_type type() const;
    bool is_null() const;
    
    String_type        get_str()    const;
    const Object*      get_obj()    const;
    const Array*       get_array()  const;
    bool               get_bool()   const;
    int                get_int()    const;
    int64              get_int64()  const;
    double             get_real()   const;
    
    Object* get_obj();
    Array*  get_array();
    
    template< typename T > T get_value() const;
    
    base::Value* DeepCopy() const;
    base::Value* get() const;
    
  private:
    void check_type( const Value_type vtype ) const;
    
    Value_type type_;
    scoped_ptr<base::Value> v_;
  };
  
  
  ///////////////////////////////////////////////////////////////////////////////////////////////
  //
  // implementation
  
  template< class tchar >
  Value_impl< tchar >::Value_impl()
  :   type_( null_type )
  {
  }
  
  template< class tchar >
  Value_impl< tchar >::Value_impl( const Const_str_ptr value )
  :   type_( str_type )
  ,   v_( new base::StringValue( srlz::internal::StringNameConvert( value ) ) )
  {
  }
  
  template< class tchar >
  Value_impl< tchar >::Value_impl( const String_type& value )
  :   type_( str_type )
  ,   v_( new base::StringValue( srlz::internal::StringNameConvert( value.c_str() ) ) )
  {
  }
  
  template< class tchar >
  Value_impl< tchar >::Value_impl( const Object* value )
  :   type_( obj_type )
  ,   v_( new base::DictionaryValue() )
  {
    if ( !value ) {
      return;
    }
    
    base::DictionaryValue * dict = nullptr;
    v_->GetAsDictionary(&dict);
    if ( !dict ) {
      return;
    }
    
    for (base::DictionaryValue::Iterator it(*value); !it.IsAtEnd(); it.Advance())
    {
      std::string key = srlz::internal::StringNameConvert( it.key().c_str() );
      dict->SetWithoutPathExpansion( key, it.value().DeepCopy() );
    }
  }
  
  template< class tchar >
  Value_impl< tchar >::Value_impl( const Array* value )
  :   type_( array_type )
  ,   v_( new base::ListValue() )
  {
    if ( !value ) {
      return;
    }
    
    base::ListValue * array = nullptr;
    v_->GetAsList(&array);
    if ( !array ) {
      return;
    }
    
    for (auto it = value->begin(); it != value->end(); ++it)
    {
      base::Value * valItem = *it;
      array->Append( valItem->DeepCopy() );
    }
  }
  
  template< class tchar >
  Value_impl< tchar >::Value_impl( bool value )
  :   type_( bool_type )
  ,   v_( new base::FundamentalValue(value) )
  {
  }
  
  template< class tchar >
  Value_impl< tchar >::Value_impl( int value )
  :   type_( int_type )
  ,   v_( new base::FundamentalValue(value) )
  {
  }
  
  template< class tchar >
  Value_impl< tchar >::Value_impl( int64 value )
  :   type_( int_type )
  ,   v_( new base::FundamentalValue(value) )
  {
  }
  
  template< class tchar >
  Value_impl< tchar >::Value_impl( double value )
  :   type_( real_type )
  ,   v_( new base::FundamentalValue(value) )
  {
  }
  
  template< class tchar >
  Value_impl< tchar >::Value_impl( const Value_impl< tchar >& other )
  :   type_( other.type() )
  {
    if ( other.v_ ) {
      v_.reset( other.v_->DeepCopy() );
    }
  }
  
  template< class tchar >
  Value_impl< tchar >::Value_impl( const base::Value* other )
  : type_(null_type)
  {
    if ( !other ) {
      return;
    }
    
    v_.reset( other->DeepCopy() );
    
    base::Value::Type tp = other->GetType();
    switch ( tp ) {
      case base::Value::TYPE_BOOLEAN:
        {
          type_ = bool_type;
        }
        break;
        
      case base::Value::TYPE_INTEGER:
        {
          type_ = int_type;
        }
        break;
        
      case base::Value::TYPE_DOUBLE:
        {
          type_ = real_type;
        }
        break;
        
      case base::Value::TYPE_STRING:
        {
          type_ = str_type;
        }
        break;
      
      case base::Value::TYPE_DICTIONARY:
        {
          type_ = obj_type;
        }
        break;
        
      case base::Value::TYPE_LIST:
        {
          type_ = array_type;
        }
        break;
        
      default:
        break;
    }
  }
  
  template< class tchar >
  Value_impl< tchar >& Value_impl< tchar >::operator=( const Value_impl& lhs )
  {
    Value_impl tmp( lhs );
    
    std::swap( type_, tmp.type_ );
    v_.swap( tmp.v_ );
    
    return *this;
  }
  
  template< class tchar >
  Value_impl< tchar >& Value_impl< tchar >::operator = ( const base::Value* lhs )
  {
    Value_impl tmp( lhs );
    
    std::swap( type_, tmp.type_ );
    v_.swap( tmp.v_ );
    
    return *this;
  }
  
  template< class tchar >
  bool Value_impl< tchar >::operator==( const Value_impl& lhs ) const
  {
    if( this == &lhs ) return true;
    
    if( type() != lhs.type() ) return false;
    
    if ( type() == null_type && lhs.type() == null_type ) {
      return true;
    }
    
    if ( type() == null_type || lhs.type() == null_type ) {
      return false;
    }
    
    return v_->Equals( lhs.v_.get() );
  }
  
  template< class tchar >
  Value_type Value_impl< tchar >::type() const
  {
    return type_;
  }
  
  template< class tchar >
  bool Value_impl< tchar >::is_null() const
  {
    return type() == null_type;
  }
  
  template< class tchar >
  void Value_impl< tchar >::check_type( const Value_type vtype ) const
  {
    if( type() != vtype )
    {
      std::ostringstream os;
      
      os << "value type is " << type() << " not " << vtype;
      
      throw std::runtime_error( os.str() );
    }
  }
  
  template< class tchar >
  typename Value_impl<tchar>::String_type Value_impl< tchar >::get_str() const
  {
    check_type(  str_type );
    
    std::string value;
    if ( v_ ) {
      v_->GetAsString(&value);
    }
    
    return srlz::internal::StringConvert<tchar>::invoke( value );
  }
  
  template< class tchar >
  const typename Value_impl< tchar >::Object* Value_impl< tchar >::get_obj() const
  {
    check_type( obj_type );
    
    base::DictionaryValue * dict = nullptr;
    v_->GetAsDictionary(&dict);    
    return dict;
  }
  
  template< class tchar >
  const typename Value_impl< tchar >::Array* Value_impl< tchar >::get_array() const
  {
    check_type(  array_type );
    
    base::ListValue * array = nullptr;
    v_->GetAsList(&array);
    return array;
  }
  
  template< class tchar >
  bool Value_impl< tchar >::get_bool() const
  {
    check_type(  bool_type );
    bool val = false;
    v_->GetAsBoolean(&val);
    return val;
  }
  
  template< class tchar >
  int Value_impl< tchar >::get_int() const
  {
    check_type(  int_type );
    
    return static_cast< int >( get_int64() );
  }
  
  template< class tchar >
  int64 Value_impl< tchar >::get_int64() const
  {
    check_type(  int_type );
    
    int64 val = 0;
    v_->GetAsInteger(&val);
    return val;
  }
  
  template< class tchar >
  double Value_impl< tchar >::get_real() const
  {
    if( type() == int_type || type() == real_type )
    {
      return static_cast< double >( get_int64() );
    }
    
    return 0;
  }
  
  template< class tchar >
  typename Value_impl< tchar >::Object* Value_impl< tchar >::get_obj()
  {
    check_type(  obj_type );
    
    base::DictionaryValue * dict = nullptr;
    v_->GetAsDictionary(&dict);
    return dict;
  }
  
  template< class tchar >
  typename Value_impl< tchar >::Array* Value_impl< tchar >::get_array()
  {
    check_type(  array_type );
    
    base::ListValue * array = nullptr;
    v_->GetAsList(&array);
    return array;
  }
  
  template< class tchar >
  base::Value* Value_impl< tchar >::DeepCopy() const
  {
    if ( !v_ ) {
      return nullptr;
    }
    return v_->DeepCopy();
  }
  
  template< class tchar >
  base::Value* Value_impl< tchar >::get() const
  {
    if ( !v_ ) {
      return nullptr;
    }
    return v_.get();
  }
  
  namespace internal_ {
    template< typename T >
    struct Type_to_type
    {
    };
    
    template< class Value >
    int get_value( const Value& value, Type_to_type< int > )
    {
      return value.get_int();
    }
    
    template< class Value >
    int64 get_value( const Value& value, Type_to_type< int64 > )
    {
      return value.get_int64();
    }
    
    template< class Value >
    double get_value( const Value& value, Type_to_type< double > )
    {
      return value.get_real();
    }
    
    template< class Value >
    typename Value::String_type get_value( const Value& value, Type_to_type< typename Value::String_type > )
    {
      return value.get_str();
    }
    
    template< class Value >
    typename Value::Array* get_value( const Value& value, Type_to_type< typename Value::Array* > )
    {
      return value.get_array();
    }
    
    template< class Value >
    typename Value::Object* get_value( const Value& value, Type_to_type< typename Value::Object* > )
    {
      return value.get_obj();
    }
    
    template< class Value >
    bool get_value( const Value& value, Type_to_type< bool > )
    {
      return value.get_bool();
    }
  }// namespace internal_
  
  template< class tchar >
  template< typename T >
  T Value_impl< tchar >::get_value() const
  {
    return internal_::get_value( *this, internal_::Type_to_type< T >() );
  }

  typedef Value_impl<char>          mValue;
  typedef Value_impl<wchar_t>       wmValue;
  
  // functions to reads a JSON values
  inline bool read( const std::string& s, mValue& value )
  {
    scoped_ptr<base::Value> val( base::JSONReader::Read(s) );
    if ( val ) {
      value = val.get();
      return true;
    }
    
    return false;
  }
  
  inline bool read( const std::wstring& s, wmValue& value )
  {
    scoped_ptr<base::Value> val( base::JSONReader::Read(base::WideToUTF8(s)) );
    if ( val ) {
      value = val.get();
      return true;
    }

    return false;
  }

  // functions to convert JSON Values to text,
  // the "formatted" versions add whitespace to format the output nicely
  inline std::string  write( const mValue& value )
  {
    std::string json;
    base::JSONWriter::WriteWithOptions(value.get(), base::JSONWriter::OPTIONS_DO_NOT_ESCAPE, &json);
    return json;
  }
  
  inline std::string  write_formatted( const mValue& value )
  {
    std::string json;
    base::JSONWriter::WriteWithOptions( value.get(), base::JSONWriter::OPTIONS_DO_NOT_ESCAPE | base::JSONWriter::OPTIONS_PRETTY_PRINT, &json );
    return json;
  }
  
  inline std::wstring write( const wmValue& value )
  {
    std::string json;
    base::JSONWriter::WriteWithOptions(value.get(), base::JSONWriter::OPTIONS_DO_NOT_ESCAPE, &json);
    return base::UTF8ToWide(json);
  }
  
  inline std::wstring write_formatted( const wmValue& value )
  {
    std::string json;
    base::JSONWriter::WriteWithOptions( value.get(), base::JSONWriter::OPTIONS_DO_NOT_ESCAPE | base::JSONWriter::OPTIONS_PRETTY_PRINT, &json );
    return base::UTF8ToWide(json);
  }
  
}// namespace json_spirit


NS_SERIALIZATION_LITE_BEGIN

// mValue
template< class tchar >
inline void serialize_load(iarchive_json const& ar, json_spirit::Value_impl<tchar>& t) {
  if ( ar.value() ) {
    t = ar.value();
  }
}

template< class tchar >
inline void serialize_save(oarchive_json& ar, json_spirit::Value_impl<tchar> const& t) {
  if ( t.get() ) {
    ar.value() = make_scoped_ptr( t.get()->DeepCopy() );
  }
}

NS_SERIALIZATION_LITE_END
