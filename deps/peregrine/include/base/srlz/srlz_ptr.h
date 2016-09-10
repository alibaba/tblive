#ifndef __SRLZ_PTR_H_0451C093_98A8_41DC_9EA4_49782A7D242C__
#define __SRLZ_PTR_H_0451C093_98A8_41DC_9EA4_49782A7D242C__

#include "./srlz_base.h"

NS_SERIALIZATION_LITE_BEGIN

template <class T>
struct ptr_creator
{
	enum {specialized = 0};
	inline static T* create()
	{
		return new T();
	}
	inline static void destroy(T* t)
	{
		delete t;
	}
};

template <typename T>
struct serializer<T*>
{
	template <template <typename, typename> class Executor, typename Archive>
	inline static void serialize(Archive& ar, T*& p)
	{
		Executor<Archive, T>::exec(ar, p, (void*)NULL);
	}
};

template <typename Archive, typename T>
struct serialize_loadptr_executor
{
	template <typename Base>
	inline static bool exec(Archive const& ar, Base*& p, void*)
	{
		if (!p)
			p = ptr_creator<T>::create();
		ar >> *p;
		return true;
	}
};

template <typename Archive, typename T>
struct serialize_saveptr_executor
{
	template <typename Base>
	inline static bool exec(Archive& ar, Base*const& p, void*)
	{
		if (p)
			ar << *p;
		return true;
	}
};

template <class Archive, class T>
inline void serialize_pointer_load(void*, Archive const& ar, T*& p)
{
	serializer<T*>::template serialize<serialize_loadptr_executor>(const_cast<Archive&>(ar), p);
}

template <class Archive, class T>
inline void serialize_pointer_save(void*, Archive& ar, T*const& p)
{
	serializer<T*>::template serialize<serialize_saveptr_executor>(ar, const_cast<T*&>(p));
}

template <class Archive, class T>
inline void serialize_load(Archive const& ar, T*& p)
{
	// 指针替换规范：serialize_load所直接或间接调用的函数，不得删除原指针，
	// 只能留给serialize_load来删除，主要是智能指针的删除方法完全不同。
	T* oldp = p;
	serialize_pointer_load((T*)0, ar, p);
	if (oldp && oldp != p)
		ptr_creator<T>::destroy(oldp);
}

template <class Archive, class T>
inline void serialize_save(Archive& ar, T*const& p)
{
	serialize_pointer_save((T*)0, ar, p);
}

//////////////////////////////////////////////////////////////////////////
// 支持指针多态的几个宏
#define SL_INHERITED_MAPPING_BEGIN(basecls) \
	namespace srlz{\
	template <> struct serializer<basecls*> \
{\
	enum {specialized = 1};\
	template <template <typename, typename> class Executor, typename Archive>\
	inline static void serialize(Archive& ar, basecls*& p)\
{\

#define SL_INHERITED_MAPPING_END() }};}

#define SL_INHERITED_ITEM_NC(name, cls) if (Executor<Archive, cls>::exec(ar, p, name)) return

#if defined(UNICODE) || defined(_UNICODE) 
#define SL_INHERITED_ITEM_C(cls) if (Executor<Archive, cls>::exec(ar, p, _LL(#cls))) return
#else
#define SL_INHERITED_ITEM_C(cls) if (Executor<Archive, cls>::exec(ar, p, #cls)) return
#endif

NS_SERIALIZATION_LITE_END
#endif//__SRLZ_PTR_H_0451C093_98A8_41DC_9EA4_49782A7D242C__
