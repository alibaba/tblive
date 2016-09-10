
#include "tblive_sdk/biz.h"
#include "tblive_sdk/scene_config.h"
#include "tblive_sdk/tblive_sdk.h"

#include "base/lazy_instance.h"
#include "base/memory/linked_ptr.h"
#include "base/callback.h"
#include "base/bind.h"
#include "base/json/json_helper.h"
#include "net/http_client/http_client_helper.h"
#include "net/base/escape.h"
#include <base/path_service.h>
#include "base/strings/string_split.h"
#include "base/strings/string_number_conversions.h"
#include "base/base_util.h"

#include "platform.hpp"
#include "task_pool.hpp"
#include "login/LoginBiz.h"


struct VersionCheckResult
{
	std::string version;
	std::string message;
	std::string downloadUrl;
};
EX_SL_CLASS_MAPPING_BEGIN(VersionCheckResult)
EX_SL_MEMBER_V(version);
EX_SL_MEMBER_V(message);
EX_SL_MEMBER_V(downloadUrl);
EX_SL_CLASS_MAPPING_END()

struct VersionCheckApiRsp
{
	VersionCheckResult version_check_get_response;
};
EX_SL_CLASS_MAPPING_BEGIN(VersionCheckApiRsp)
EX_SL_MEMBER_V(version_check_get_response);
EX_SL_CLASS_MAPPING_END()

namespace {

const char* UPDATE_CHECK_URL = "http://jdy.tmall.com/version/check";

int StringVersion2NumbetVersion(std::string version)
{
	// 1.01.00
	std::vector<std::string> verNums;
	base::SplitString(version, '.', &verNums);
	if ( verNums.size() == 3 )
	{
		int major = 0, minor = 0, build = 0;

		base::StringToInt(verNums[0], &major);
		base::StringToInt(verNums[1], &minor);
		base::StringToInt(verNums[2], &build);

		return major * 10000 + minor * 100 + build;
	}

	return 0;
}

std::string BaseValueToString( base::Value * val, bool pretty = false ) 
{
	if ( !val )	
	{
		return "";
	}

	std::string json;
	int options = base::JSONWriter::OPTIONS_DO_NOT_ESCAPE;
	if (pretty)
	{
		options = options | base::JSONWriter::OPTIONS_PRETTY_PRINT;
	}
	
	base::JSONWriter::WriteWithOptions(val, options, &json);
	
	return json;
}

// 场景管理
class SceneConfigMgr
{
public:
	SceneConfigMgr() 
	{
		scene_source_.baked = true;
		scene_source_.id = SCENE_DEF_ID;
		scene_source_.name = SCENE_DEF_NAME;
	}

	void Clear()
	{
		scene_source_.settings.reset();
		sources_.clear();
	}

	void GetSourceProperties(std::string source_id, scoped_ptr<base::Value> & properties)
	{
		tblive_sdk::get_property_from_source_default(source_id, properties);
		if (properties)
		{// 每次都是从源刷新，同时覆盖缓存
			source_property_map_[source_id] = make_linked_ptr(properties->DeepCopy());
		}
	}

	bool SetVideoSelected(std::string id)
	{
		// "dshow_input"
		linked_ptr<base::Value> pps = source_property_map_[SOURCE_DSHOW_INPUT];
		if ( pps.get() )
		{
			DShowInputProperties items;
			json::JsonToClass(make_scoped_ptr(pps->DeepCopy()), items);
			
			auto fit = std::find_if(items.video_device_id.begin(), items.video_device_id.end(), 
				[id](PropertyListItem item) {
				if ( item.plist_name == id )
				{
					return true;
				}
				return false;
			});
			if ( fit != items.video_device_id.end()  )
			{
				// Add source
				SourceVideoSetting dshowSetting;
				std::string devide_id;
				devide_id = fit->plist_value;
				dshowSetting.video_device_id = devide_id;
				dshowSetting.last_video_device_id = devide_id;
				
				SourceConfig dshowSource;
				dshowSource.id = SOURCE_DSHOW_INPUT;
				dshowSource.name = SOURCE_DSHOW_INPUT;
				dshowSource.flags = 1;
				dshowSource.volume = 0;
				scoped_ptr<base::Value> spSetting = json::JsonFromClass(dshowSetting);
				if ( spSetting )
				{
					dshowSource.settings = make_linked_ptr(spSetting->DeepCopy());
				}
				sources_.push_back(dshowSource);

				// Add to scene_source setting
				SceneSettingItem scene_item;
				scene_item.align = 5;
				scene_item.name = SOURCE_DSHOW_INPUT;

				SourceSceneSetting scene_setting;
				if (scene_source_.settings.get())
				{
					json::JsonToClass(make_scoped_ptr(scene_source_.settings->DeepCopy()), scene_setting);
				}
				scene_setting.items.push_back(scene_item);
				scoped_ptr<base::Value> spSceneSetting = json::JsonFromClass(scene_setting);
				if ( spSceneSetting )
				{
					scene_source_.settings = make_linked_ptr(spSceneSetting->DeepCopy());
				}				

				return true;
			}
		}

		// "decklink-input"
		pps = source_property_map_[SOURCE_OBS_BMC_CAPTURE];
		if (pps.get())
		{
			DeckLinkProperties items;
			json::JsonToClass(make_scoped_ptr(pps->DeepCopy()), items);

			auto fit = std::find_if(items.device_list.begin(), items.device_list.end(),
				[id](PropertyListItem item) {
				if (item.plist_name == id)
				{
					return true;
				}
				return false;
			});
			if (fit != items.device_list.end())
			{
				// Add source
				SourceDeckLinkSetting decklinkSetting;
				std::string device_id;
				device_id = fit->plist_value;
				decklinkSetting.device_list = device_id;

				std::string device_mode;
				for ( auto item : items.mode_list )
				{
					// 1080i50
					if ( item.plist_name == "1080i50" )
					{
						device_mode = item.plist_value;
						break;
					}
				}

				decklinkSetting.mode_list = device_mode;

				SourceConfig decklinkSource;
				decklinkSource.id = SOURCE_OBS_BMC_CAPTURE;
				decklinkSource.name = SOURCE_OBS_BMC_CAPTURE;
				decklinkSource.flags = 0;
				decklinkSource.mixers = 15;
				decklinkSource.volume = 0;

				scoped_ptr<base::Value> spSetting = json::JsonFromClass(decklinkSetting);
				if (spSetting)
				{
					decklinkSource.settings = make_linked_ptr(spSetting->DeepCopy());
				}
				sources_.push_back(decklinkSource);

				// Add to scene_source setting
				SceneSettingItem scene_item;
				scene_item.align = 5;
				scene_item.name = SOURCE_OBS_BMC_CAPTURE;

				SourceSceneSetting scene_setting;
				if (scene_source_.settings.get())
				{
					json::JsonToClass(make_scoped_ptr(scene_source_.settings->DeepCopy()), scene_setting);
				}
				scene_setting.items.push_back(scene_item);
				scoped_ptr<base::Value> spSceneSetting = json::JsonFromClass(scene_setting);
				if (spSceneSetting)
				{
					scene_source_.settings = make_linked_ptr(spSceneSetting->DeepCopy());
				}

				return true;
			}
		}

		return false;
	}

	bool SetAudioInputSelected(std::string id)
	{
		linked_ptr<base::Value> pps = source_property_map_[SOURCE_WASAPI_INPUT_CAPTURE];
		if (pps.get())
		{
			AudioDeviceProperties items;
			json::JsonToClass(make_scoped_ptr(pps->DeepCopy()), items);

			auto fit = std::find_if(items.device_id.begin(), items.device_id.end(),
				[id](PropertyListItem item) {
				if (item.plist_name == id)
				{
					return true;
				}
				return false;
			});
			if (fit != items.device_id.end())
			{
				// Add source
				SourceAduioInputSetting audioInputSetting;
				std::string devide_id;
				devide_id = fit->plist_value;
				audioInputSetting.device_id = devide_id;

				SourceConfig audioInputSource;
				audioInputSource.id = SOURCE_WASAPI_INPUT_CAPTURE;
				audioInputSource.name = SOURCE_WASAPI_INPUT_CAPTURE;
				audioInputSource.mixers = 15;
				audioInputSource.volume = 1.0;
				scoped_ptr<base::Value> spSetting = json::JsonFromClass(audioInputSetting);
				if (spSetting)
				{
					audioInputSource.settings = make_linked_ptr(spSetting->DeepCopy());
				}
				sources_.push_back(audioInputSource);

				// Add to scene_source setting
				SceneSettingItem scene_item;
				scene_item.align = 5;
				scene_item.name = SOURCE_WASAPI_INPUT_CAPTURE;

				SourceSceneSetting scene_setting;
				if (scene_source_.settings.get())
				{
					json::JsonToClass(make_scoped_ptr(scene_source_.settings->DeepCopy()), scene_setting);
				}
				scene_setting.items.push_back(scene_item);
				scoped_ptr<base::Value> spSceneSetting = json::JsonFromClass(scene_setting);
				if (spSceneSetting)
				{
					scene_source_.settings = make_linked_ptr(spSceneSetting->DeepCopy());
				}

				return true;
			}
		}

		return false;
	}

	std::string GetSceneConfig()
	{
		SceneConfig scene_config;
		scene_config.sources.push_back(scene_source_);
		std::copy(sources_.begin(), sources_.end(), std::back_inserter(scene_config.sources));

		return json::JsonStrFromClass(scene_config, true);
	}

private:
	// 当前的场景配置用于生成json配置
	SourceConfig scene_source_;
	std::vector<SourceConfig> sources_;

	// 缓存源的属性
	std::map<std::string, linked_ptr<base::Value> > source_property_map_;
};

base::LazyInstance<SceneConfigMgr> g_scene_config_mgr = LAZY_INSTANCE_INITIALIZER;

}// namespace

namespace biz {

void removeCurrentScene()
{
	tblive_sdk::remove_current_scene();
	g_scene_config_mgr.Get().Clear();
}

std::string getVideoList()
{
	scoped_ptr<base::Value> videoList(new base::ListValue());
	base::ListValue * vlist = nullptr;
	videoList->GetAsList(&vlist);
	if (!vlist) {
		return "[]";
	}

	// dshow
	scoped_ptr<base::Value> dshowProperties;
	g_scene_config_mgr.Get().GetSourceProperties(SOURCE_DSHOW_INPUT, dshowProperties);
	if ( dshowProperties )
	{
		DShowInputProperties items;
		json::JsonToClass(dshowProperties, items);
		for (auto device : items.video_device_id)
		{
			vlist->Append(new base::StringValue(device.plist_name));
		}
	}

	// blackmagic
	scoped_ptr<base::Value> blackmagicProperties;
	g_scene_config_mgr.Get().GetSourceProperties(SOURCE_OBS_BMC_CAPTURE, blackmagicProperties);
 	if (blackmagicProperties)
	{
		DeckLinkProperties items;
		json::JsonToClass(blackmagicProperties, items);
		for ( auto device : items.device_list )
		{
			vlist->Append(new base::StringValue(device.plist_name));
		}
	}

	return BaseValueToString(videoList.get());
}

bool setVideoSelected(std::string id)
{
	return g_scene_config_mgr.Get().SetVideoSelected(id);
}

std::string getAudioInputList()
{
	scoped_ptr<base::Value> audioInputList(new base::ListValue());
	base::ListValue * vlist = nullptr;
	audioInputList->GetAsList(&vlist);
	if (!vlist) {
		return "[]";
	}

	// audio
	scoped_ptr<base::Value> audioInpitProperties;
	g_scene_config_mgr.Get().GetSourceProperties(SOURCE_WASAPI_INPUT_CAPTURE, audioInpitProperties);
	if (audioInpitProperties)
	{
		//std::string strDebug;
		//base::JSONWriter::Write(audioInpitProperties.get(), &strDebug);
		//std::wstring wstrDebug = base::UTF8ToWide(strDebug);
		AudioDeviceProperties items;
		json::JsonToClass(audioInpitProperties, items);
		for (auto it = items.device_id.begin(); it != items.device_id.end(); ++it)
		{
			vlist->Append(new base::StringValue(it->plist_name));
		}
	}

	return BaseValueToString(audioInputList.get());
}

void getAudioDeviceInfoList(std::vector<PropertyListItem> &oAudioDeviceInfos, const std::string &strDeviceType)
{
	oAudioDeviceInfos.clear();
	// audio
	scoped_ptr<base::Value> audioInpitProperties;
	g_scene_config_mgr.Get().GetSourceProperties(strDeviceType.c_str(), audioInpitProperties);
	if (audioInpitProperties)
	{
		//这里可以调试查看详细的属性信息
		//std::string strDebug;
		//base::JSONWriter::Write(audioInpitProperties.get(), &strDebug);
		//std::wstring wstrDebug = base::UTF8ToWide(strDebug);

		AudioDeviceProperties items;
		json::JsonToClass(audioInpitProperties, items);
		oAudioDeviceInfos = items.device_id;
	}
}

bool setAudioInputSelected(std::string id)
{
	return g_scene_config_mgr.Get().SetAudioInputSelected(id);
}


int getVolume()
{
	return tblive_sdk::get_volume();
}

void setVolume(int vol)
{
	tblive_sdk::set_volume(vol);
}

std::string getSceneConfig()
{
	return g_scene_config_mgr.Get().GetSceneConfig();
}

void loadSceneConfig(std::string config)
{
	tblive_sdk::load_scene_config(config);
}

void setRtmpUrl(std::string rtmp_url)
{
	ServiceConfig config;
	config.settings.upload_url = rtmp_url;

	std::string json = json::JsonStrFromClass(config, true);
	tblive_sdk::service_set_config(json);
}


bool startStreaming(std::function<void(int,std::wstring)> callback)
{
	return tblive_sdk::start_streaming(callback);
}

void stopStreaming()
{
	tblive_sdk::stop_streaming();
}

void dispatchTBLiveCmd(std::wstring cmd, std::map<std::wstring, std::wstring> args)
{
	tblive_sdk::dispatch_tblive_cmd(cmd, args);
}

int getStreamimgStatus()
{
	bool active = tblive_sdk::streamimg_active();
	return active ? 1 : 0;
}

void checkTBliveUpdate()
{
	task_pool::PostTask(base::Lambda([](){
		// config
		base::FilePath exepath;
		PathService::Get(base::FILE_EXE, &exepath);
		base::FilePath path = exepath.DirName().DirName().DirName().Append("data/obs-studio/version_config.json");

		std::map<std::string, std::string> config;
		json::JsonFileToClass(base::FilePath(path), config);

		// Check
		std::string stringVersion = config["VersionString"];
		if ( stringVersion.empty() )
		{
			return;
		}

		std::string nick;
		PrgString strUser = CLoginBiz::GetInstance()->GetLoginedUser();
		if ( strUser.IsEmpty() )
		{
			return;
		}
		nick = base::WideToUTF8(strUser.GetString());
		
		std::string url = UPDATE_CHECK_URL;
		url += "?";
		url += "version=";
		url += stringVersion;

		url += "&nick=";
		url += net::EscapeQueryParamValue(nick, true);

		url += "&platform=tblive";

		net::HttpRequest req;
		req.method = L"GET";
		req.url = base::UTF8ToWide(url);
		net::HttpResponse rsp;
		net::SendRequest(req, 10 * 1000, rsp);
		
		VersionCheckApiRsp apiRsp;
		json::JsonStrToClass(rsp.GetDataInUnicode(), apiRsp);
		if ( apiRsp.version_check_get_response.version.empty() || 
			apiRsp.version_check_get_response.downloadUrl.empty() )
		{
			return;
		}

		// Compare version
		if ( StringVersion2NumbetVersion(apiRsp.version_check_get_response.version) > 
			StringVersion2NumbetVersion(stringVersion) )
		{
			std::string ver = apiRsp.version_check_get_response.version;
			std::string msg = apiRsp.version_check_get_response.message;
			std::string url = apiRsp.version_check_get_response.downloadUrl;

			base::GetUIMessageLoop()->PostTask(FROM_HERE, base::Lambda([=](){
				tblive_sdk::tblive_need_update(ver, msg, url);
			}));
		}
	}));
}

void onTaoBaoTokenOk()
{
	tblive_sdk::on_taotao_token_ok();
}

void setSourceConfig(std::wstring sourceConfigJson)
{
	tblive_sdk::set_cource_config(sourceConfigJson);
}

void removeSceneItemFromSource(const char* name)
{
	tblive_sdk::remove_scene_item_from_source(name);
}

}// namespace biz