#ifndef __XML_ADDONS_H_416C2EF3_EB0D_4326_9115_CC450ED1BC27__
#define __XML_ADDONS_H_416C2EF3_EB0D_4326_9115_CC450ED1BC27__

#include "./srlz_base.h"

//////////////////////////////////////////////////////////////////////////
// xml_to_map_key addon
namespace srlz
{
	namespace addon
	{
		template <class T>
		struct xml_to_map_key
		{
			static inline char const* name(char) {return "id";}
			static inline wchar_t const* name(wchar_t) {return L"id";}
			static inline bool is_attr() {return true;}
		};
	}
}
#define ADDON_XML_TO_MAP_KEY(clsname, keyname, isattr) \
	namespace srlz\
	{\
		namespace addon\
		{\
			template <> struct xml_to_map_key<clsname>\
			{\
				static inline char const* name(char) {return #keyname;}\
				static inline wchar_t const* name(wchar_t) {return _LL(#keyname);}\
				static inline bool is_attr() {return isattr;}\
			};\
		}\
	}\


//////////////////////////////////////////////////////////////////////////
// xml_mapping_nodetype addon
namespace srlz
{
	namespace addon
	{
		struct xml_mapping_nodetype
		{
			struct xmn_text;
			struct xmn_attr;
			struct xmn_cdata;
		};
		template <typename OwnerType, size_t offset>
		struct xml_mapping
		{
			typedef xml_mapping_nodetype::xmn_text nodetype;
		};
	}
}

#define ADDON_XML_MAPPING_NODETYPE(cls, member, ntype) \
	namespace srlz\
	{\
		namespace addon\
		{\
			template<> struct xml_mapping<cls, offsetof(cls, member)>\
			{\
				typedef xml_mapping_nodetype::ntype nodetype;\
			};\
		}\
	}\

//////////////////////////////////////////////////////////////////////////
// xml_inhertied_tag
namespace srlz
{
	namespace addon
	{
		enum xml_inhertied_tag_type
		{
			xitt_attr,
			xitt_element,
			xitt_self,
		};
		template <class BaseClass>
		struct xml_inherited_tag
		{
			static inline char const* name(char) {return "type";}
			static inline wchar_t const* name(wchar_t) {return L"type";}
			static inline xml_inhertied_tag_type type() {return xitt_attr;}
		};
	}
}
#define ADDON_XML_INHERITED_TAG(basecls, tagtype, tagname) \
	namespace srlz\
	{\
		namespace addon\
		{\
			template <> struct xml_inherited_tag<basecls>\
			{\
				static inline char const* name(char) {return #tagname;}\
				static inline wchar_t const* name(wchar_t) {return _LL(#tagname);}\
				static inline xml_inhertied_tag_type type() {return xitt_ ## tagtype;}\
			};\
		}\
	}\


#endif//__XML_ADDONS_H_416C2EF3_EB0D_4326_9115_CC450ED1BC27__
