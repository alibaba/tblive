/********************************************************************
	created:	2007/04/28
	created:	28:4:2007   7:48
	filename: 	Sources\Modules\serialization\srlz_base.h
	author:		shaoyoushi	
	purpose:	提供序列化的最基本的功能，也是这套序列化库的根本原理。
				序列化的原理大体是这样：每个需要序列化的类型，都需要
				提供序列化函数。可以是两个分开的函数，分别用来输入和
				输出；也可以是一个函数同时完成输入和输出（推荐）。
				序列化函数可以是这个类型的成员函数serialize，也可以是
				全局函数serialize_load和serialize_save，还可以通过特化
				的serializer<>来实现。
				对于每种序列化目标数据格式来说，需要实现最基本两个档案
				类，一个是iarchive_xxx，一个是oarchive_xxx，分别从
				iarchive_base<>和oarichive_base继承而来；还需要实现这种
				档案类型对基础类型的序列化函数和对member_nvio类型的序列化
				函数。
				序列化的流程从某个archive的输入输出或者&操作符开始，
				先寻找被序列化对象有没有实现serialize_load/serialize_save
				的函数重载，如果有，则调用之。如果没有，则由泛化的
				serialize_load或者serialize_save模版函数将调用转发到
				serializer模版类，看看被序列化对象有没有实现serializer
				的特化，如果有，则调用之，如果没有则由泛化serializer将
				调用转发到被序列化对象的serialize成员函数，如果这个函数
				存在，则调用之，否则序列化失败，编译器报错。
*********************************************************************/
#ifndef __SRLZ_BASE_H_FD9DE0CE_AE8A_43B7_A93B_E6F7E3C2A6CC__
#define __SRLZ_BASE_H_FD9DE0CE_AE8A_43B7_A93B_E6F7E3C2A6CC__

#define NS_SERIALIZATION_LITE_BEGIN	namespace srlz{
#define NS_SERIALIZATION_LITE_END	}


NS_SERIALIZATION_LITE_BEGIN

//struct SerializeOption
//{
//
//	enum OptContFlag 
//	{
//		ocf_replace = 0, 
//		ocf_append = 1, 
//		ocf_appendnew = 2,
//		ocf_overwrite = 3,
//	};
//	enum OptPtrFlag
//	{
//		opf_matchtype = 0,
//		opf_notmatchtype = 1,
//	};
//	OptContFlag contoption : 2;
//	OptPtrFlag ptroption : 1;
//
//	SerializeOption()
//	{
//		*reinterpret_cast<long*>(this) = 0;
//	}
//	SerializeOption(SerializeOption const& that)
//	{
//		*reinterpret_cast<long*>(this) = *reinterpret_cast<long*>(&that);
//	}
//	SerializeOption& operator=(SerializeOption const& rhs)
//	{
//		*reinterpret_cast<long*>(this) = *reinterpret_cast<long*>(&rhs);
//		return this;
//	}
//};

//////////////////////////////////////////////////////////////////////////
// 泛化的序列化操作类，需要被序列化的类型可以选择通过特化serializer来实现
// 自己的序列化函数。参数T就是需要被序列化的类型，内部的Archive模版参数是
// 序列化的档案类型。
// 如果编译器报错定位到这个地方,提示serialize不是某个类的成员,说明这个类
// 没有实现其序列化函数
template <typename T> 
struct serializer
{
	enum {specialized = 0};
	template <class Archive>
	inline static void serialize(Archive& ar, T& t)
	{
		t.serialize(ar);
	}
};

//////////////////////////////////////////////////////////////////////////
// 泛化的serialize_load和serialize_save函数，需要被序列化的类型可以选择通过
// 重载这两个函数来实现序列化，其中参数ar是序列化的档案，t是被序列化的对象
// serialize_load函数实现将对象t从档案ar中读取出来，serialize_save函数实现
// 将对象t保存到档案ar中。
template <class Archive, class T>
inline void serialize_load(Archive const& ar, T& t)
{
	serializer<T>::serialize(const_cast<Archive&>(ar), t);
}

template <class Archive, class T>
inline void serialize_save(Archive& ar, T const& t)
{
	serializer<T>::serialize(ar, const_cast<T&>(t));
}


//////////////////////////////////////////////////////////////////////////
// 输入档案的基类，所有输入档案都由这个类派生，Archive模版参数就是具体的
// 派生类。
template <class Archive>
class iarchive_base
{
public:

	// 输入输出方向，用来帮助判断档案是输入还是输出的
	struct direction
	{
		enum {value = 1};
	};

	// 输入操作符重载，调用的是serialize_load
	template <class T>
	inline const Archive& operator>> (T const& t) const
	{
		serialize_load(*This(), const_cast<T&>(t));
		return *This();
	}

	// 使用&作为通用序列化操作符，对于输入档案，它和>>是一样的功能
	template <class T> 
	inline const Archive& operator& (T const& t) const
	{
		serialize_load(*This(), const_cast<T&>(t));
		return *This();
	}

protected:

	// 辅助函数，内部使用
	inline const Archive* This() const
	{
		return static_cast<Archive const*>(this);
	}
};

//////////////////////////////////////////////////////////////////////////
// 输出档案的基类，所有输出档案都由这个类派生，Archive模版参数就是具体的
// 派生类。
template <class Archive>
class oarchive_base
{
public:

	// 输入输出方向，用来帮助判断档案是输入还是输出的
	struct direction
	{
		enum {value = 0};
	};

	// 输出操作符重载，调用的是serialize_save
	template<class T>
	inline Archive& operator<< (const T& t)
	{
		serialize_save(*This(), t);
		return *This();
	}

	// 使用&作为通用序列化操作符，对于输出档案，它和<<是一样的功能
	template<class T> 
	inline Archive& operator& (const T& t)
	{
		serialize_save(*This(), t);
		return *This();
	}

protected:

	// 辅助函数，内部使用
	inline Archive* This()
	{
		return static_cast<Archive*>(this);
	}
};


#define __LL(x) L ## x
#define _LL(x) __LL(x)
#define _ANULL ((const char*)0)
#define _WNULL ((const wchar_t*)0)

#if defined(UNICODE) || defined(_UNICODE)
#define _TNULL _WNULL
#else
#define _TNULL _ANULL
#endif

NS_SERIALIZATION_LITE_END

#endif//__SRLZ_BASE_H_FD9DE0CE_AE8A_43B7_A93B_E6F7E3C2A6CC__
