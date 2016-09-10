#ifndef __SRLZ_MEMBER_H_451F0E00_B3DC_4AB1_A333_F1AD0A54AC93__
#define __SRLZ_MEMBER_H_451F0E00_B3DC_4AB1_A333_F1AD0A54AC93__

#include "./srlz_base.h"

NS_SERIALIZATION_LITE_BEGIN

#ifndef SRLZ_MEMBER_FORCE_UNICODE
#define SRLZ_MEMBER_FORCE_UNICODE 1
#endif

//////////////////////////////////////////////////////////////////////////
// 用来描述一个序列化成员的结构，描述了序列化时的名、值、初始值、附加信息等。可以被用在成员变量、普通变量、基类等多种地方。
template <typename OwnerType, size_t offset, typename CharType, typename ValueType, typename InitType>
struct member_nvio
{
	inline member_nvio(CharType const* n, ValueType& v, InitType const& i) : name(n), value(v), ival(i) {}
	inline member_nvio(member_nvio<OwnerType, offset, CharType, ValueType, InitType>const& r) : name(r.name), value(r.value), ival(r.ival) {}
	CharType const* name;
	ValueType& value;
	InitType const& ival;
};

template <typename OwnerType, size_t offset, typename CharType, typename ValueType, typename InitType>
inline member_nvio<OwnerType, offset, CharType, ValueType, InitType> make_member_nvio(CharType const* n, ValueType& v, InitType const& i)
{
	return member_nvio<OwnerType, offset, CharType, ValueType, InitType>(n, v, i);
}

class null_owner;
class special_owner;
class null_init_type {};

#define SL_SINGLEA_V(v) ::srlz::make_member_nvio<srlz::null_owner, 0>(#v, v, srlz::null_init_type())
#define SL_SINGLEW_V(v) ::srlz::make_member_nvio<srlz::null_owner, 0>(_LL(#v), v, srlz::null_init_type())
#define SL_SINGLE_NV(n, v) ::srlz::make_member_nvio<srlz::null_owner, 0>(n, v, srlz::null_init_type())
#define SL_SINGLEA_ITEM(v) ::srlz::make_member_nvio<srlz::null_owner, 0>("item", v, srlz::null_init_type())
#define SL_SINGLEW_ITEM(v) ::srlz::make_member_nvio<srlz::null_owner, 0>(L"item", v, srlz::null_init_type())


#define SL_CLASS_MAPPING_BEGIN(cls) \
	template <typename Archive> void serialize(Archive& ar) {\
	typedef cls this_class; \

#define SL_CLASS_MAPPING_END()	}

#define SL_MEMBERA_V(v) ar & ::srlz::make_member_nvio<this_class, offsetof(this_class, v)>(#v, v, srlz::null_init_type())
#define SL_MEMBERW_V(v) ar & ::srlz::make_member_nvio<this_class, offsetof(this_class, v)>(_LL(#v), v, srlz::null_init_type())

#define SL_MEMBERA_VI(v, i) ar & ::srlz::make_member_nvio<this_class, offsetof(this_class, v)>(#v, v, i)
#define SL_MEMBERW_VI(v, i) ar & ::srlz::make_member_nvio<this_class, offsetof(this_class, v)>(_LL(#v), v, i)

#define SL_MEMBER_NV(n, v) ar & ::srlz::make_member_nvio<this_class, offsetof(this_class, v)>(n, v, srlz::null_init_type())

#define SL_MEMBER_NVI(n, v, i) ar & ::srlz::make_member_nvio<this_class, offsetof(this_class, v)>(n, v, i)

#define SL_BASEA(base_cls) ar & ::srlz::make_member_nvio<this_class, (size_t)-1>(_ANULL, static_cast<base_cls&>(*this), static_cast<base_cls&>(*this))
#define SL_BASEW(base_cls) ar & ::srlz::make_member_nvio<this_class, (size_t)-1>(_WNULL, static_cast<base_cls&>(*this), static_cast<base_cls&>(*this))

#define EX_SL_CLASS_MAPPING_BEGIN(cls) \
	namespace srlz\
	{\
		template <> struct serializer<cls>\
		{\
			enum {specialized = 1};\
			template <class Archive> inline static void serialize(Archive& ar, cls& t)\
			{\
				typedef cls this_class;\

#define EX_SL_CLASS_MAPPING_END() }};}

#define EX_SL_MEMBERA_V(v) ar & ::srlz::make_member_nvio<this_class, offsetof(this_class, v)>(#v, t.v, srlz::null_init_type())
#define EX_SL_MEMBERW_V(v) ar & ::srlz::make_member_nvio<this_class, offsetof(this_class, v)>(_LL(#v), t.v, srlz::null_init_type())

#define EX_SL_MEMBERA_VI(v, i) ar & ::srlz::make_member_nvio<this_class, offsetof(this_class, v)>(#v, t.v, i)
#define EX_SL_MEMBERW_VI(v, i) ar & ::srlz::make_member_nvio<this_class, offsetof(this_class, v)>(_LL(#v), t.v, i)

#define EX_SL_MEMBER_NV(n, v) ar & ::srlz::make_member_nvio<this_class, offsetof(this_class, v)>(n, t.v, srlz::null_init_type())

#define EX_SL_MEMBER_NVI(n, v, i) ar & ::srlz::make_member_nvio<this_class, offsetof(this_class, v)>(n, t.v, i)

#define EX_SL_BASEA(base_cls) ar & ::srlz::make_member_nvio<this_class, (size_t)-1>(_ANULL, static_cast<base_cls&>(t), static_cast<base_cls&>(t))
#define EX_SL_BASEW(base_cls) ar & ::srlz::make_member_nvio<this_class, (size_t)-1>(_WNULL, static_cast<base_cls&>(t), static_cast<base_cls&>(t))

#if defined(UNICODE) || defined(_UNICODE) || (SRLZ_MEMBER_FORCE_UNICODE == 1)

#define SL_SINGLE_V			SL_SINGLEW_V
#define SL_SINGLE_ITEM		SL_SINGLEW_ITEM
#define SL_MEMBER_V			SL_MEMBERW_V
#define SL_MEMBER_VI		SL_MEMBERW_VI
#define SL_BASE				SL_BASEW
#define EX_SL_MEMBER_V		EX_SL_MEMBERW_V
#define EX_SL_MEMBER_VI		EX_SL_MEMBERW_VI
#define EX_SL_BASE			EX_SL_BASEW

#else

#define SL_SINGLE_V			SL_SINGLEA_V
#define SL_SINGLE_ITEM		SL_SINGLEA_ITEM
#define SL_MEMBER_V			SL_MEMBERA_V
#define SL_MEMBER_VI		SL_MEMBERA_VI
#define SL_BASE				SL_BASEA
#define EX_SL_MEMBER_V		EX_SL_MEMBERA_V
#define EX_SL_MEMBER_VI		EX_SL_MEMBERA_VI
#define EX_SL_BASE			EX_SL_BASEA

#endif

#define EX_SL_ACCESS_CLASS(cls) friend ::srlz::serializer<cls>

#define SL_SPLIT_CLASS_MAPPING(cls)	\
	template <typename Archive, typename direction> struct serialize_splitter;\
	template <typename Archive> struct serialize_splitter<Archive, typename ::srlz::iarchive_base<Archive>::direction>\
	{\
		inline static void exec(cls* pThis, Archive& ar)\
		{\
			pThis->serialize_load(ar);\
		}\
	};\
	template <typename Archive> struct serialize_splitter<Archive, typename ::srlz::oarchive_base<Archive>::direction>\
	{\
		inline static void exec(cls* pThis, Archive& ar)\
		{\
			pThis->serialize_save(ar);\
		}\
	};\
	template <typename Archive> inline void serialize(Archive& ar) \
	{\
		serialize_splitter<Archive, Archive::direction>::exec(this, ar);\
	}\


class iarchive_members_initializer : public iarchive_base<iarchive_members_initializer>
{
};

template <typename OwnerType, size_t offset, typename CharType, typename ValueType, typename InitType>
inline void serialize_load(const iarchive_members_initializer& ar, member_nvio<OwnerType, offset, CharType, ValueType, InitType>& member)
{
	member.value = member.ival;
}

template <typename OwnerType, size_t offset, typename CharType, typename ValueType>
inline void serialize_load(const iarchive_members_initializer& ar, member_nvio<OwnerType, offset, CharType, ValueType, null_init_type>& member)
{
}

class iarchive_members_initializer2 : public iarchive_base<iarchive_members_initializer2>
{
};

template <typename OwnerType, size_t offset, typename CharType, typename ValueType, typename InitType>
inline void serialize_load(const iarchive_members_initializer2& ar, member_nvio<OwnerType, offset, CharType, ValueType, InitType>& member)
{
	member.value = member.ival;
}

template <int serializer_specialized> 
struct iarchive_members_initializer2_executor
{
	template <typename T>
	inline static void exec(const iarchive_members_initializer2& ar, T& t)
	{
		ar & t;
	};
};

template <> 
struct iarchive_members_initializer2_executor<0>
{
	template <typename T>
	inline static void exec(const iarchive_members_initializer2& ar, T& t)
	{
	}
};

template <typename OwnerType, size_t offset, typename CharType, typename ValueType>
inline void serialize_load(const iarchive_members_initializer2& ar, member_nvio<OwnerType, offset, CharType, ValueType, null_init_type>& member)
{
	iarchive_members_initializer2_executor<serializer<ValueType>::specialized>::exec(ar, member.value);
}

template <typename T>
inline void init_members(T& t)
{
	srlz::iarchive_members_initializer() & t;
}

template <typename T>
inline void init_members_r(T& t)
{
	srlz::iarchive_members_initializer2() & t;
}

#define SL_CTOR_INIT()	init_members(*this);


#define EX_SL_ENUM_ASINT(enum_type) \
	namespace srlz\
	{\
		template <> struct serializer<enum_type>\
		{\
			template <class Archive> inline static void serialize(Archive& ar, enum_type& t)\
			{\
				ar & reinterpret_cast<int&>(t);\
			}\
		};\
	}\


NS_SERIALIZATION_LITE_END
#endif//__SRLZ_MEMBER_H_451F0E00_B3DC_4AB1_A333_F1AD0A54AC93__
