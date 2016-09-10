#ifndef __SRLZ_SMARTPTR_H_B1E139F7_07BE_4D8F_9AF8_AC2BCDE55F42__
#define __SRLZ_SMARTPTR_H_B1E139F7_07BE_4D8F_9AF8_AC2BCDE55F42__

#include "./srlz_base.h"
#include "./srlz_ptr.h"
#include <memory>
#include <base/memory/linked_ptr.h>
#include <base/memory/scoped_ptr.h>
#include <base/memory/ref_counted.h>

NS_SERIALIZATION_LITE_BEGIN

//////////////////////////////////////////////////////////////////////////
// std::auto_ptr
template <class Archive, class T>
inline void serialize_load(Archive const& ar, std::auto_ptr<T>& sp)
{
	// 指针替换规范：serialize_load所直接或间接调用的函数，不得删除原指针，
	// 只能留给serialize_load来删除，主要是智能指针的删除方法完全不同。
    T* p = sp.get();
    serialize_pointer_load((T*)0, ar, p);
	if (sp.get() != p)
	{
		ptr_creator<T>::destroy(sp.get());
		sp.reset(p);
	}
}

template <class Archive, class T>
inline void serialize_save(Archive& ar, std::auto_ptr<T> const& sp)
{
	serialize_pointer_save((T*)0, ar, sp.get());
}

////////////////////////////////////////////////////////////////////////
// linked_ptr
template <long Specialized> struct shared_ptr_replacer;
template<> struct shared_ptr_replacer<1>
{
	template <typename T>
	inline static void exec(linked_ptr<T>& sp, T* p)
	{
		sp.reset(p, ptr_creator<T>::destroy);
	}
};
template<> struct shared_ptr_replacer<0>
{
	template <typename T>
	inline static void exec(linked_ptr<T>& sp, T* p)
	{
		sp.reset(p);
	}
};
template <class Archive, class T>
inline void serialize_load(Archive const& ar, linked_ptr<T>& sp)
{
	// 指针替换规范：serialize_load所直接或间接调用的函数，不得删除原指针，
	// 只能留给serialize_load来删除，主要是智能指针的删除方法完全不同。
	T* p = sp.get();
	serialize_pointer_load((T*)0, ar, p);
	if (sp.get() != p)
		shared_ptr_replacer<ptr_creator<T>::specialized>::exec(sp, p);
}

template <class Archive, class T>
inline void serialize_save(Archive& ar, linked_ptr<T>const& sp)
{
	serialize_pointer_save((T*)0, ar, sp.get());
}

//////////////////////////////////////////////////////////////////////////
// scoped_refptr
template <class Archive, class T>
inline void serialize_load(Archive const& ar, scoped_refptr<T>& sp)
{
	// 指针替换规范：serialize_load所直接或间接调用的函数，不得删除原指针，
	// 只能留给serialize_load来删除，主要是智能指针的删除方法完全不同。
	T* p = sp.get();
	serialize_pointer_load((T*)0, ar, p);
	if (sp.get() != p)
		sp.swap(scoped_refptr<T>(p, false));
}

template <class Archive, class T>
inline void serialize_save(Archive& ar, scoped_refptr<T>const& sp)
{
	serialize_pointer_save((T*)0, ar, sp.get());
}

//////////////////////////////////////////////////////////////////////////
// scoped_ptr
template <class Archive, class T>
inline void serialize_load(Archive const& ar, scoped_ptr<T>& sp)
{
	// 指针替换规范：serialize_load所直接或间接调用的函数，不得删除原指针，
	// 只能留给serialize_load来删除，主要是智能指针的删除方法完全不同。
	T* p = sp.get();
	serialize_pointer_load((T*)0, ar, p);
	if (sp.get() != p)
	{
		ptr_creator<T>::destroy(sp.get());
		sp.reset(p);
	}
}

template <class Archive, class T>
inline void serialize_save(Archive& ar, scoped_ptr<T>const& sp)
{
	serialize_pointer_save((T*)0, ar, sp.get());
}


NS_SERIALIZATION_LITE_END
#endif//__SRLZ_SMARTPTR_H_B1E139F7_07BE_4D8F_9AF8_AC2BCDE55F42__
