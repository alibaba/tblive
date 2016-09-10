#pragma once

#include "base/memory/scoped_ptr.h"
#include "base/values.h"

#include "obs.hpp"

#include <string>
#include <functional>

/*
tblive_sdk:
基于libobs内核和obsUI层，封装操作场景、输入源、属性的方法，供业务层调用

关于场景切换：基于动态生成json配置，然后调用load_scene_config来实现

*/

namespace tblive_sdk {

// 设置UI根对象
void set_obs_basic(void * obs_basic);

// 加载基于json配置的场景
void load_scene_config( std::string jsonConfig );

// 清理当前的场景
void remove_current_scene();

// 添加场景
bool add_scene(std::string name);

// 选择场景
void select_scene(std::string name);

// 添加源（暂时不需要，对源的增删操作改为对场景json的load操作）
// 注意：obs会回调弹出添加源的属性界面
bool add_source_to_scene(std::string source_id);

// 查找源
obs_source_t * get_source_by_id(std::string source_id);

// 从场景项（对应某个源）获取属性
void get_property_from_source_default(std::string source_id, scoped_ptr<base::Value>& propertiesValue);
void get_property_from_source(OBSSource source, scoped_ptr<base::Value>& propertiesValue);
void get_property_from_scene_item(OBSSceneItem item, scoped_ptr<base::Value>& propertiesValue);

// 获取、设置音量（0 ~ 100）
int get_volume();
void set_volume(int vol);

// 固化所有源
//void scene_bake_sources();

// 设置推流地址
void service_set_config(std::string config);

// 开始/停止推流
bool start_streaming(std::function<void(int, std::wstring)> callback);
void stop_streaming();

// 业务webview访问url
void dispatch_tblive_cmd(std::wstring cmd, std::map<std::wstring, std::wstring> args);

// 是否推流状态
bool streamimg_active();

// 检测到tblive需要升级
void tblive_need_update(std::string ver, std::string msg, std::string download_url);

void on_taotao_token_ok();

void set_cource_config(std::wstring sourceConfigJson);

//add by weiqi.mwq
//更改源的settings属性
void adjust_properties_settings_for_source(obs_source_t * source, const char *settingsKey, const char *settingsVal);

//添加源到当前场景
OBSSource add_source_to_current_scene(const std::string &source_id, const std::string &source_name, const bool bForceAdd = false);

//添加源到指定场景
OBSSource add_source_to_scene(OBSScene	destScene, const std::string &source_id, const std::string &source_name, const bool bForceAdd = false);

//添加本地音频设备源到当前场景
void check_add_all_audio_sources_to_current_scene();

//添加本地音频设备源到指定场景
void check_add_all_audio_sources_to_scene(OBSScene scene);

void remove_scene_item_from_source(const char* name);

}// namespace tblive_sdk

