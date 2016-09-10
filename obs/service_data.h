#pragma once

#include <string>
#include <vector>

#include "base/json/json_helper.h"


// webview service
struct ServiceItem
{
	std::wstring biz;
	std::wstring service;
	std::wstring main_url;
};
EX_SL_CLASS_MAPPING_BEGIN(ServiceItem)
EX_SL_MEMBER_V(biz);
EX_SL_MEMBER_V(service);
EX_SL_MEMBER_V(main_url);
EX_SL_CLASS_MAPPING_END()

struct ServiceData
{
	std::vector<ServiceItem> service_list;
};
EX_SL_CLASS_MAPPING_BEGIN(ServiceData)
EX_SL_MEMBER_V(service_list);
EX_SL_CLASS_MAPPING_END()



