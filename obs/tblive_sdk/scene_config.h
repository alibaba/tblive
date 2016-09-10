#pragma once

#include <string>
#include <vector>
#include "base/memory/linked_ptr.h"

#include "base/json/json_helper.h"

#define SCENE_DEF_ID					"scene"
#define SCENE_DEF_NAME					"tblive"

// 摄像头
#define SOURCE_DSHOW_INPUT				"dshow_input"

// blackmagic 采集卡
#define SOURCE_OBS_BMC_CAPTURE			"decklink-input"

// 音频输入（麦克风）
#define SOURCE_WASAPI_INPUT_CAPTURE		"wasapi_input_capture"

// 音频输出（电脑输出设备，如听筒，用于捕捉如游戏，播放音乐等）
#define SOURCE_WASAPI_OUTPUT_CAPTURE	"wasapi_output_capture"

// 场景和源的配置，用于obs场景应用 ( 参照scene.json )

struct SourceConfig 
{
	SourceConfig(): baked(true), flags(0), mixers(0), sync(0), volume(1.0) {}

    bool baked;
    int flags;
    std::string id;
    int mixers;
    std::string name;
    linked_ptr<base::Value> settings;
    int sync;
    double volume;
};
EX_SL_CLASS_MAPPING_BEGIN(SourceConfig)
EX_SL_MEMBER_V(baked);
EX_SL_MEMBER_V(flags);
EX_SL_MEMBER_V(id);
EX_SL_MEMBER_V(mixers);
EX_SL_MEMBER_V(name);
EX_SL_MEMBER_V(settings);
EX_SL_MEMBER_V(sync);
EX_SL_MEMBER_V(volume);
EX_SL_CLASS_MAPPING_END()

struct SceneConfig 
{
	SceneConfig() : current_scene(SCENE_DEF_NAME) {}

    std::string current_scene;
    std::vector<SourceConfig> sources;
};
EX_SL_CLASS_MAPPING_BEGIN(SceneConfig)
EX_SL_MEMBER_V(current_scene);
EX_SL_MEMBER_V(sources);
EX_SL_CLASS_MAPPING_END()


struct SceneSettingPos 
{
	SceneSettingPos() : x(0), y(0) {}
	SceneSettingPos(double vx, double vy) : x(vx), y(vy) {}

	double x;
	double y;	
};
EX_SL_CLASS_MAPPING_BEGIN(SceneSettingPos)
EX_SL_MEMBER_V(x);
EX_SL_MEMBER_V(y);
EX_SL_CLASS_MAPPING_END()

struct SceneSettingItem 
{
	SceneSettingItem() 
    : align(5)
    , bounds(SceneSettingPos(1280, 720))
    , bounds_align(0)
    , bounds_type(2)
    , pos(SceneSettingPos(0, 0))
    , rot(0)
    , scale(SceneSettingPos(1.0, 1.0))
    , visible(true)
	{}

	int align;
	// 默认分辨率1280*720，bounds相对应
	SceneSettingPos bounds; 
	int bounds_align;
	int bounds_type;
	std::string name;
	SceneSettingPos pos;
	double rot;
	SceneSettingPos scale;
	bool visible;
};
EX_SL_CLASS_MAPPING_BEGIN(SceneSettingItem)
EX_SL_MEMBER_V(align);
EX_SL_MEMBER_V(bounds);
EX_SL_MEMBER_V(bounds_align);
EX_SL_MEMBER_V(bounds_type);
EX_SL_MEMBER_V(name);
EX_SL_MEMBER_V(pos);
EX_SL_MEMBER_V(rot);
EX_SL_MEMBER_V(scale);
EX_SL_MEMBER_V(visible);
EX_SL_CLASS_MAPPING_END()


struct SourceSceneSetting 
{
	std::vector<SceneSettingItem> items;
};
EX_SL_CLASS_MAPPING_BEGIN(SourceSceneSetting)
EX_SL_MEMBER_V(items);
EX_SL_CLASS_MAPPING_END()


struct SourceVideoSetting 
{
	SourceVideoSetting() : active(true), res_type(0), ui_action(false) {}

	bool active;
	std::string last_resolution;
	std::string last_video_device_id;
	int res_type;
	std::string resolution;
	bool ui_action;
	std::string video_device_id;

};
EX_SL_CLASS_MAPPING_BEGIN(SourceVideoSetting)
EX_SL_MEMBER_V(active);
EX_SL_MEMBER_V(last_resolution);
EX_SL_MEMBER_V(last_video_device_id);
EX_SL_MEMBER_V(res_type);
EX_SL_MEMBER_V(resolution);
EX_SL_MEMBER_V(ui_action);
EX_SL_MEMBER_V(video_device_id);
EX_SL_CLASS_MAPPING_END()


struct SourceDeckLinkSetting
{
	SourceDeckLinkSetting() : ui_action(false) {}

	std::string device_list;
	std::string mode_list;
	bool ui_action;
};
EX_SL_CLASS_MAPPING_BEGIN(SourceDeckLinkSetting)
EX_SL_MEMBER_V(device_list);
EX_SL_MEMBER_V(mode_list);
EX_SL_MEMBER_V(ui_action);
EX_SL_CLASS_MAPPING_END()

struct SourceAduioInputSetting
{
	SourceAduioInputSetting() : ui_action(false) {}

	std::string device_id;
	bool ui_action;
};
EX_SL_CLASS_MAPPING_BEGIN(SourceAduioInputSetting)
EX_SL_MEMBER_V(device_id);
EX_SL_MEMBER_V(ui_action);
EX_SL_CLASS_MAPPING_END()


// 源的属性配置（基于插件内部get_properties的实现） 
// obs中源的属性界面元素就是基于这个配置

struct PropertyListItem
{
	std::string plist_name;
	std::string plist_value;
};
EX_SL_CLASS_MAPPING_BEGIN(PropertyListItem)
EX_SL_MEMBER_V(plist_name);
EX_SL_MEMBER_V(plist_value);
EX_SL_CLASS_MAPPING_END()


struct DShowInputProperties
{
	std::vector<PropertyListItem> buffering;
	std::vector<PropertyListItem> color_range;
	std::vector<PropertyListItem> color_space;
	std::vector<PropertyListItem> frame_interval;
	std::vector<PropertyListItem> res_type;
	std::vector<PropertyListItem> resolution;
	std::vector<PropertyListItem> video_device_id;
	std::vector<PropertyListItem> video_format;
};
EX_SL_CLASS_MAPPING_BEGIN(DShowInputProperties)
EX_SL_MEMBER_V(buffering);
EX_SL_MEMBER_V(color_range);
EX_SL_MEMBER_V(color_space);
EX_SL_MEMBER_V(frame_interval);
EX_SL_MEMBER_V(res_type);
EX_SL_MEMBER_V(resolution);
EX_SL_MEMBER_V(video_device_id);
EX_SL_MEMBER_V(video_format);
EX_SL_CLASS_MAPPING_END()


struct DeckLinkProperties
{
	DeckLinkProperties() : deinterlace(true), full_range(false) {}

	bool deinterlace;
	std::vector<PropertyListItem> device_list;
	bool full_range;
	std::vector<PropertyListItem> mode_list;
};
EX_SL_CLASS_MAPPING_BEGIN(DeckLinkProperties)
EX_SL_MEMBER_V(deinterlace);
EX_SL_MEMBER_V(device_list);
EX_SL_MEMBER_V(full_range);
EX_SL_MEMBER_V(mode_list);
EX_SL_CLASS_MAPPING_END()


struct AudioDeviceProperties
{
	AudioDeviceProperties() : use_device_timing(true) {}
	std::vector<PropertyListItem> device_id;
	bool use_device_timing;
};
EX_SL_CLASS_MAPPING_BEGIN(AudioDeviceProperties)
EX_SL_MEMBER_V(device_id);
EX_SL_MEMBER_V(use_device_timing);
EX_SL_CLASS_MAPPING_END()


// 推流服务
struct ServiceSetting 
{
	std::string upload_url;
};
EX_SL_CLASS_MAPPING_BEGIN(ServiceSetting)
EX_SL_MEMBER_V(upload_url);
EX_SL_CLASS_MAPPING_END()


struct ServiceConfig 
{
	ServiceConfig() : type("rtmp_custom") {}

	ServiceSetting settings;
	std::string type;
};
EX_SL_CLASS_MAPPING_BEGIN(ServiceConfig)
EX_SL_MEMBER_V(settings);
EX_SL_MEMBER_V(type);
EX_SL_CLASS_MAPPING_END()

