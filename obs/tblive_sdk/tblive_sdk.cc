
#include "tblive_sdk/tblive_sdk.h"
#include "tblive_sdk/scene_config.h"
#include "window-basic-main.hpp"

#include "obs.hpp"
#include "obs-app.hpp"

#include <util/dstr.h>
#include <util/util.hpp>
#include <util/platform.h>

#include <memory>
#include "base/base_util.h"
#include "base/bind.h"
#include "base/memory/ref_counted.h"
#include "tblive_sdk/biz.h"
#include "obs.hpp"
#include "tblive_sdk/tblive_sdk.h"
#include "tblive_sdk/scene_config.h"

namespace {

static OBSBasic * g_obs_basic_main = nullptr;

void adjust_properties_for_source(obs_source_t * source)
{
	std::string source_id = obs_source_get_id(source);

	// Get setting of source
	OBSData settings = obs_source_get_settings(source);
	obs_data_release(settings);

	if ( source_id == "dshow_input" )
	{
		scoped_ptr<base::Value> spProperty;
		tblive_sdk::get_property_from_source(source, spProperty);

		DShowInputProperties items;
		json::JsonToClass(spProperty, items);

		// 视频捕获需要设置具体的video_device_id值，才会出现对应的分辨率resolution列表
		// 默认选择1280*720，这样屏幕能撑满
		auto fit = std::find_if(items.resolution.begin(), items.resolution.end(), [](PropertyListItem item)
		{
			if (item.plist_name == "1280x720")
			{
				return true;
			}
			return false;
		});
		if (fit != items.resolution.end())
		{
			obs_data_set_int(settings, "res_type", 1);
			obs_data_set_string(settings, "last_resolution", "1280x720");
			obs_data_set_string(settings, "resolution", "1280x720");
			obs_source_update(source, settings);
		}
	}
}

float mul_to_db(const float mul)
{
    return (mul == 0.0f) ? -INFINITY : (20.0f * log10f(mul));
}

float db_to_mul(const float db)
{
    return isfinite((double)db) ? powf(10.0f, db / 20.0f) : 0.0f;
}

}// namespace


namespace tblive_sdk {

void set_obs_basic(void * obs_basic)
{
	g_obs_basic_main = (OBSBasic *)obs_basic;
	if ( g_obs_basic_main )
	{
		base::InitUIMessageLoop();
	}
}

void load_scene_config( std::string jsonConfig ) 
{
	if (!g_obs_basic_main)
	{
		return;
	}

	BPtr<char> jsonData = bstrdup_n(jsonConfig.data(), jsonConfig.size());
	if (!jsonData) {
		return;
	}

	obs_data_t       *data = obs_data_create_from_json(jsonData);
	obs_data_array_t *sources = obs_data_get_array(data, "sources");
	const char       *sceneName = obs_data_get_string(data,
		"current_scene");
	obs_source_t     *curScene;

    obs_load_sources(sources, OBSBasic::SourceLoaded, g_obs_basic_main);

	curScene = obs_get_source_by_name(sceneName);
	obs_set_output_source(0, curScene);
	obs_source_release(curScene);

	obs_data_array_release(sources);
	obs_data_release(data);

	{// Delay adjust properties
		auto delay_adjust_properties = [](obs_scene_t *scene, obs_sceneitem_t *item,
			void *param)
		{
			OBSSource source = obs_sceneitem_get_source(item);
			adjust_properties_for_source(source);

			return true;
		};

		obs_scene_enum_items(g_obs_basic_main->GetCurrentScene(), delay_adjust_properties, nullptr);
	}
}

void remove_current_scene() 
{
	if ( !g_obs_basic_main )
	{
		return;
	}

	OBSScene scene = g_obs_basic_main->GetCurrentScene();
	obs_source_t *source = obs_scene_get_source(scene);
	if (source) {
		obs_source_remove(source);
	}
}

bool add_scene(std::string name)
{
	obs_source_t *source = obs_get_source_by_name(name.c_str());
	if (source) {
		return false;
	}

	obs_scene_t *scene = obs_scene_create(name.c_str());
	source = obs_scene_get_source(scene);

	if ( g_obs_basic_main )
	{
		g_obs_basic_main->AddScene(source);
		//g_obs_basic_main->SetCurrentScene(source);
	}

	obs_scene_release(scene);
	return true;
}

void select_scene(std::string name)
{
	obs_source_t *source = obs_get_source_by_name(name.c_str());
	if (source) {
		obs_scene_t  *scene = obs_scene_from_source(source);

		if ( g_obs_basic_main )
		{
			g_obs_basic_main->SetCurrentScene(scene, false);
		}
	}
}

void get_property_from_source(OBSSource source, scoped_ptr<base::Value>& propertiesValue)
{
	propertiesValue.reset(new base::DictionaryValue());
	base::DictionaryValue * dict = nullptr;
	propertiesValue->GetAsDictionary(&dict);
	if (!dict) {
		return;
	}

	using properties_delete_t = decltype(&obs_properties_destroy);
	using properties_t =
		std::unique_ptr < obs_properties_t, properties_delete_t > ;

	std::string source_id = obs_source_get_id(source);

	// Get setting of source
	OBSData settings = obs_source_get_settings(source);
	obs_data_release(settings);

	properties_t properties(nullptr, obs_properties_destroy);
	properties.reset(obs_source_properties(source));

	obs_property_t *property = obs_properties_first(properties.get());

	while (property) {
		const char* name = obs_property_name(property);
		obs_property_type type = obs_property_get_type(property);

		if (!obs_property_visible(property))
			return;

		switch (type) {
		case OBS_PROPERTY_INVALID:
			return;

		case OBS_PROPERTY_BOOL:
		{
			bool bVal = obs_data_get_bool(settings, name);
			dict->SetWithoutPathExpansion(name, new base::FundamentalValue(bVal));
			break;
		}

		case OBS_PROPERTY_INT:
		{
			int intVal = (int)obs_data_get_int(settings, name);
			dict->SetWithoutPathExpansion(name, new base::FundamentalValue(intVal));
			break;
		}

		case OBS_PROPERTY_FLOAT:
		{
			double fVal = obs_data_get_double(settings, name);
			dict->SetWithoutPathExpansion(name, new base::FundamentalValue(fVal));
			break;
		}

		case OBS_PROPERTY_TEXT:
		{
			const char * strVal = obs_data_get_string(settings, name);
			dict->SetWithoutPathExpansion(name, new base::StringValue(strVal));
			break;
		}

		case OBS_PROPERTY_PATH:
			// 			AddPath(property, layout, &label);
			break;

		case OBS_PROPERTY_LIST:
		{
			obs_combo_format format = obs_property_list_format(property);
			size_t           count = obs_property_list_item_count(property);

			base::ListValue * listValue = nullptr;
			if (count > 0)
			{
				listValue = new base::ListValue;
				dict->SetWithoutPathExpansion(name, listValue);
			}

			std::string backupStrVal;
			for (size_t i = 0; i < count; i++)
			{
				const char *list_name = obs_property_list_item_name(property, i);
				base::DictionaryValue item;
				item.SetWithoutPathExpansion("plist_name", new base::StringValue(list_name));


				if (format == OBS_COMBO_FORMAT_INT) {
					long long llVal = obs_property_list_item_int(property, i);
					item.SetWithoutPathExpansion("plist_value", new base::FundamentalValue(llVal));
					listValue->Append(item.DeepCopy());

				}
				else if (format == OBS_COMBO_FORMAT_FLOAT) {
					double dVal = obs_property_list_item_float(property, i);
					item.SetWithoutPathExpansion("plist_value", new base::FundamentalValue(dVal));
					listValue->Append(item.DeepCopy());

				}
				else if (format == OBS_COMBO_FORMAT_STRING) {
					std::string strVal = obs_property_list_item_string(property, i);
					if (backupStrVal.empty())
					{
						backupStrVal = strVal;
					}
					item.SetWithoutPathExpansion("plist_value", new base::StringValue(strVal));
					listValue->Append(item.DeepCopy());
				}
			}
			
			break;
		}

		case OBS_PROPERTY_COLOR:
			// 			AddColor(property, layout, label);
			break;

		case OBS_PROPERTY_FONT:
			// 			AddFont(property, layout, label);
			break;

		case OBS_PROPERTY_BUTTON:
			// 			widget = AddButton(property);
			break;
		}

		obs_property_next(&property);
	}
}

void get_property_from_scene_item(OBSSceneItem item, scoped_ptr<base::Value>& propertiesValue)
{
	OBSSource source = obs_sceneitem_get_source(item);
	get_property_from_source(source, propertiesValue);
}

void get_property_from_source_default(std::string source_id, scoped_ptr<base::Value>& propertiesValue)
{
	obs_source_t *source = obs_source_create(source_id.c_str(), "inner_used_for_get_property", nullptr, nullptr);
	if ( source )
	{
		get_property_from_source(source, propertiesValue);

		// 创建source，读取属性，然后马上关闭
		// 同步去关闭，在 blackmagic 的情况下会崩溃，改成异步
		obs_source_release(source);
		//base::GetUIMessageLoop()->PostTask(FROM_HERE, base::Bind(&obs_source_release, source));
	}
}

bool add_source_to_scene(std::string source_id)
{
	// Add source by API, set source name same with source_id by default 
	const char *name = source_id.c_str();

	obs_source_t *source = obs_get_output_source(0);
	obs_scene_t  *scene = obs_scene_from_source(source);
	bool         success = false;
	if (!source)
		return false;

	source = obs_get_source_by_name(name);
	if (source) {
		return false;
	}
	else {
		source = obs_source_create(source_id.c_str(), name, nullptr, nullptr);
		
    // TODO: 
		//set the auto-baked value here
		//obs_source_set_baked(source, App()->autoBaked());

		if (source) {
            obs_scene_atomic_update(scene, [](void *data, obs_scene_t *scene){
                obs_source_t *source = (obs_source_t*)data;

                obs_sceneitem_t *sceneitem = obs_scene_add(scene, source);
                obs_sceneitem_set_visible(sceneitem, true);
            }, source);

			success = true;
		}
	}

	obs_source_release(source);
	obs_scene_release(scene);

	return success;
}

obs_source_t * get_source_by_id(std::string source_id)
{
	obs_source_t * source = nullptr;
	std::pair<obs_source_t **, std::string> pr = std::make_pair(&source, source_id);

	auto callback = [](void* data, obs_source_t* s) {
		std::pair<obs_source_t **, std::string> & pr =
			*(std::pair<obs_source_t **, std::string>*)(data);
		obs_source_t ** pSource = pr.first;
		std::string sid = pr.second;

		const char *id = obs_source_get_id(s);

		if (strcmp(id, sid.c_str()) == 0)
		{
			*pSource = s;
			obs_source_addref(s);
			return false;
		}

		// continue
		return true;
	};

	obs_enum_sources(callback, &pr);

	return source;
}

int get_volume()
{
	int vol = 100;

	if (!g_obs_basic_main)
	{
		return vol;
	}

	OBSScene scene = g_obs_basic_main->GetCurrentScene();
	obs_scene_enum_items(scene, [](obs_scene_t *scene, obs_sceneitem_t *item, void *param)
	{
		int * pVol = (int*)param;
		obs_source_t *source = obs_sceneitem_get_source(item);
		if (source)
		{
			int& vol = *pVol;
			uint32_t flags = obs_source_get_output_flags(source);
			if (flags & OBS_SOURCE_AUDIO)
			{
				float db = mul_to_db(obs_source_get_volume(source));
				vol = (int)( ((db + 120) * 100) / 120 );
				return false;
			}
		}
		return true;
	}, &vol);

	return vol;
}

void set_volume(int vol)
{
	if (!g_obs_basic_main)
	{
		return;
	}

	OBSScene scene = g_obs_basic_main->GetCurrentScene();
	obs_scene_enum_items(scene, [](obs_scene_t *scene, obs_sceneitem_t *item, void *param)
	{
		int * pVol = (int*)param;
		obs_source_t *source = obs_sceneitem_get_source(item);
		if (source)
		{
			int vol = *pVol;
			uint32_t flags = obs_source_get_output_flags(source);
			if (flags & OBS_SOURCE_AUDIO)
			{
				float db = (120.0 * vol) / 100 - 120;
				obs_source_set_volume(source, db_to_mul(db));
			}
		}
		return true;
	}, &vol);
}

// void scene_bake_sources()
// {
// 	if (!g_obs_basic_main)
// 	{
// 		return;
// 	}
// 
// 	OBSScene scene = g_obs_basic_main->GetCurrentScene();
// 	obs_scene_enum_items(scene, [](obs_scene_t *scene, obs_sceneitem_t *item, void *param) 
// 	{
// 		obs_source_t *source = obs_sceneitem_get_source(item);
// 		if (source)
// 		{
// 			if (!obs_source_is_baked(source))
// 			{
// 				obs_source_set_baked(source, true);
// 			}
// 		}
// 		return true;
// 	}, NULL);
// }

void service_set_config(std::string config)
{
	if (!g_obs_basic_main)
	{
		return;
	}

	g_obs_basic_main->ReloadServiceFromConfig(config);
}

bool start_streaming(std::function<void(int, std::wstring)> callback)
{
	if (!g_obs_basic_main)
	{
		return false;
	}

    // TODO
    return false;
// 	return g_obs_basic_main->StartStreamingCmd(callback);
}

void stop_streaming()
{
	if (!g_obs_basic_main)
	{
		return;
	}

    // TODO
// 	g_obs_basic_main->StopStreamingCmd();
}


void dispatch_tblive_cmd(std::wstring cmd, std::map<std::wstring, std::wstring> args)
{
	if (g_obs_basic_main)
	{
		g_obs_basic_main->DispatchTBLiveCmd(cmd, args);
	}
}

bool streamimg_active()
{
	if ( g_obs_basic_main )
	{
		return g_obs_basic_main->StreamingActive();
	}

	return false;
}

void tblive_need_update(std::string ver, std::string msg, std::string download_url)
{
	if ( g_obs_basic_main )
	{
		g_obs_basic_main->TBLiveNeedUpdate(ver, msg, download_url);
	}
}

void on_taotao_token_ok()
{
	if ( g_obs_basic_main )
	{
		g_obs_basic_main->OnTaoBaoTokenOk();
	}
}

void set_cource_config(std::wstring sourceConfigJson)
{
	if (g_obs_basic_main)
	{
		g_obs_basic_main->SetSourceConfig(sourceConfigJson);
	}
}

void adjust_properties_settings_for_source(obs_source_t * source, const char *settingsKey, const char *settingsVal)
{
	OBSData settings = obs_source_get_settings(source);
	if (settings)
	{
		obs_data_set_string(settings, settingsKey, settingsVal);
		obs_source_update(source, settings);
		obs_data_release(settings);
	}
	else
	{
		_ASSERT(false);
	}
}

OBSSource add_source_to_current_scene(const std::string &source_id, const std::string &source_name, const bool bForceAdd/* = false*/)
{
	OBSSource sourcePtr = nullptr;
	OBSBasic	*main = reinterpret_cast<OBSBasic*>(App()->GetMainWindow());
	OBSScene	destScene = main->GetCurrentScene();
	if (!destScene)
	{
		return nullptr;
	}

	obs_source_t *source = obs_get_source_by_name(source_name.c_str());
	if (source)
	{
		//强制添加时，若已经创建过同名源，则使用之前创建的,非强制添加时，直接返回失败
		if (!bForceAdd)
		{
			return nullptr;
		}
	}
	else
	{
		source = obs_source_create(source_id.c_str(), source_name.c_str(), nullptr, nullptr);
	}

	if (source)
	{
		obs_scene_atomic_update(destScene, [](void *data, obs_scene_t *scene){
			obs_source_t *source = (obs_source_t*)data;

			obs_sceneitem_t *sceneitem = obs_scene_add(scene, source);
			obs_sceneitem_set_visible(sceneitem, true);
		}, source);

		sourcePtr = source;
	}

	//scene出了作用域，自己会递减引用计数的
	obs_source_release(source);

	return sourcePtr;
}

OBSSource add_source_to_scene(OBSScene	destScene, const std::string &source_id, const std::string &source_name, const bool bForceAdd/* = false*/)
{
	OBSSource sourcePtr = nullptr;
	if (!destScene)
	{
		return nullptr;
	}

	obs_source_t *source = obs_get_source_by_name(source_name.c_str());
	if (source)
	{
		//强制添加时，若已经创建过同名源，则使用之前创建的,非强制添加时，直接返回失败
		if (!bForceAdd)
		{
			return nullptr;
		}
	}
	else
	{
		source = obs_source_create(source_id.c_str(), source_name.c_str(), nullptr, nullptr);
	}

	if (source)
	{
		obs_scene_atomic_update(destScene, [](void *data, obs_scene_t *scene){
			obs_source_t *source = (obs_source_t*)data;

			obs_sceneitem_t *sceneitem = obs_scene_add(scene, source);
			obs_sceneitem_set_visible(sceneitem, true);
		}, source);

		sourcePtr = source;
	}

	//scene出了作用域，自己会递减引用计数的
	obs_source_release(source);

	return sourcePtr;
}
bool is_source_name_exist_in_curent_scean(std::wstring wstrSourceName)
{
	//当前仅在向场景添加源的时候，判断同名源是否已经存在，避免冲突，当前已经可以通过obs_get_source_by_name来判断所以，暂时可以不用实现
	//TODO：参考界面逻辑，获取当前场景下的所有源
	return false;
}
bool is_source_name_exist(std::wstring wstrSourceName)
{
	bool bExist = false;
	obs_source_t *source = obs_get_source_by_name(base::WideToUTF8(wstrSourceName).c_str());
	if (source)
	{
		obs_source_release(source);
		bExist = true;
	}
	
	return bExist;
}
std::wstring get_current_scean_name()
{
	OBSBasic	*main = reinterpret_cast<OBSBasic*>(App()->GetMainWindow());
	if (main)
	{
		OBSScene	scene = main->GetCurrentScene();
		if (scene)
		{
			return base::UTF8ToWide(obs_source_get_name(obs_scene_get_source((obs_scene_t*)scene)));
		}
	}

	_ASSERT(false);
	return L"";
}

void check_add_all_audio_sources_to_current_scene()
{
	std::vector<PropertyListItem> oAudioDeviceInfos;

	//weiqi.mwq：使用场景名作为前缀，场景更改名后也是会乱套，所以应该在场景改名时，把所有的本地input、output设备源都同步改名字前缀
	const std::wstring wstrSourceNamePrefix = get_current_scean_name() + L": ";
	std::string deviceIdValue;//设备唯一性标示

	//1.麦克音频
	
	//获取所有麦克设备的列表
	biz::getAudioDeviceInfoList(oAudioDeviceInfos, SOURCE_WASAPI_INPUT_CAPTURE);

	std::vector<PropertyListItem>::iterator itAudioDevice;

	for (itAudioDevice = oAudioDeviceInfos.begin(); 
		itAudioDevice != oAudioDeviceInfos.end(); itAudioDevice++)
	{
		deviceIdValue = itAudioDevice->plist_value;
		//过滤掉默认的音频设备
		if (deviceIdValue != "default")
		{
			std::wstring wstrSourceName = wstrSourceNamePrefix + base::UTF8ToWide(itAudioDevice->plist_name);

			if (!is_source_name_exist(wstrSourceName))
			{
				OBSSource obsSource = add_source_to_current_scene(SOURCE_WASAPI_INPUT_CAPTURE, base::WideToUTF8(wstrSourceName));
				if (obsSource != nullptr)
				{
					adjust_properties_settings_for_source(obsSource, "device_id", deviceIdValue.c_str());

					//新添加的都默认置为静音
					obs_source_set_muted(obsSource, true);
				}
			}
		}
	}

	//2.扬声器音频

	//获取所有扬声器设备的列表
	biz::getAudioDeviceInfoList(oAudioDeviceInfos, SOURCE_WASAPI_OUTPUT_CAPTURE);

	for (itAudioDevice = oAudioDeviceInfos.begin();
		itAudioDevice != oAudioDeviceInfos.end(); itAudioDevice++)
	{
		deviceIdValue = itAudioDevice->plist_value;
		//过滤掉默认的音频设备
		if (deviceIdValue != "default")
		{
			std::wstring wstrSourceName = wstrSourceNamePrefix + base::UTF8ToWide(itAudioDevice->plist_name);
			if (!is_source_name_exist(wstrSourceName))
			{
				OBSSource obsSource = add_source_to_current_scene(SOURCE_WASAPI_OUTPUT_CAPTURE, base::WideToUTF8(wstrSourceName));
				if (obsSource != nullptr)
				{
					adjust_properties_settings_for_source(obsSource, "device_id", deviceIdValue.c_str());
				}

				//新添加的都默认置为静音
				obs_source_set_muted(obsSource, true);
			}
		}
	}
}


void check_add_all_audio_sources_to_scene(OBSScene scene)
{
	if (scene == nullptr)
	{
		return;
	}

	std::vector<PropertyListItem> oAudioDeviceInfos;

	//weiqi.mwq：使用场景名作为前缀，场景更改名后也是会乱套，所以应该在场景改名时，把所有的本地input、output设备源都同步改名字前缀
	const std::wstring wstrSourceNamePrefix = base::UTF8ToWide(obs_source_get_name(obs_scene_get_source((obs_scene_t*)scene))) + L": ";
	std::string deviceIdValue;//设备唯一性标示

	//1.麦克音频

	//获取所有麦克设备的列表
	biz::getAudioDeviceInfoList(oAudioDeviceInfos, SOURCE_WASAPI_INPUT_CAPTURE);

	std::vector<PropertyListItem>::iterator itAudioDevice;

	for (itAudioDevice = oAudioDeviceInfos.begin();
		itAudioDevice != oAudioDeviceInfos.end(); itAudioDevice++)
	{
		deviceIdValue = itAudioDevice->plist_value;
		//过滤掉默认的音频设备
		if (deviceIdValue != "default")
		{
			std::wstring wstrSourceName = wstrSourceNamePrefix + base::UTF8ToWide(itAudioDevice->plist_name);

			if (!is_source_name_exist(wstrSourceName))
			{
				OBSSource obsSource = add_source_to_scene(scene,SOURCE_WASAPI_INPUT_CAPTURE, base::WideToUTF8(wstrSourceName));
				if (obsSource != nullptr)
				{
					adjust_properties_settings_for_source(obsSource, "device_id", deviceIdValue.c_str());

					//新添加的都默认置为静音
					obs_source_set_muted(obsSource, true);
				}
			}
		}
	}

	//2.扬声器音频

	//获取所有扬声器设备的列表
	biz::getAudioDeviceInfoList(oAudioDeviceInfos, SOURCE_WASAPI_OUTPUT_CAPTURE);

	for (itAudioDevice = oAudioDeviceInfos.begin();
		itAudioDevice != oAudioDeviceInfos.end(); itAudioDevice++)
	{
		deviceIdValue = itAudioDevice->plist_value;
		//过滤掉默认的音频设备
		if (deviceIdValue != "default")
		{
			std::wstring wstrSourceName = wstrSourceNamePrefix + base::UTF8ToWide(itAudioDevice->plist_name);
			if (!is_source_name_exist(wstrSourceName))
			{
				OBSSource obsSource = add_source_to_scene(scene,SOURCE_WASAPI_OUTPUT_CAPTURE, base::WideToUTF8(wstrSourceName));
				if (obsSource != nullptr)
				{
					adjust_properties_settings_for_source(obsSource, "device_id", deviceIdValue.c_str());
				}

				//新添加的都默认置为静音
				obs_source_set_muted(obsSource, true);
			}
		}
	}
}

void remove_scene_item_from_source(const char* name)
{
	if ( g_obs_basic_main )
	{
		g_obs_basic_main->RemoveSceneItemFromSource(name);
	}
}

}// namespace tblive_sdk
