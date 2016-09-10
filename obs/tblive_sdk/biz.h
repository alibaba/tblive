#pragma once

#include <string>
#include <functional>
#include <map>
#include <vector>
#include "tblive_sdk/scene_config.h"

/*
biz:
	封装给前端业务调用的方法
*/


namespace biz {

// 删除当前场景
void removeCurrentScene();


/*
* 获取视频列表，Video包含：
*	dshow_input：摄像头
*	decklink-input：blackmagic摄像机
* 返回json表示的设备列表:
*/
std::string getVideoList();


// 设置video
bool setVideoSelected(std::string id);


// 获取音频输入列表，Audio
std::string getAudioInputList();

// 获取音频输入列表，包括设备名、设备标示
void getAudioDeviceInfoList(std::vector<PropertyListItem> &oAudioDeviceInfos, const std::string &strDeviceType);

// 设置audio input
bool setAudioInputSelected(std::string id);


// 获取设置音量
int getVolume();
void setVolume(int vol);


// 获取总的用户设置
std::string getSceneConfig();


// 加载场景配置
void loadSceneConfig(std::string config);


// 设置推流地址
void setRtmpUrl(std::string rtmp_url);


// 开始/停止 串流
bool startStreaming(std::function<void(int, std::wstring)> callback);
void stopStreaming();


// 发起tblive协议命令
void dispatchTBLiveCmd(std::wstring cmd, std::map<std::wstring, std::wstring> args);


// 查看当前推流状态: 1表示在推流；0表示没有推流
int getStreamimgStatus();

// 检测升级
void checkTBliveUpdate();

// 淘宝免登陆token准备好了
void onTaoBaoTokenOk();

// js接口：设置部分源可用
void setSourceConfig(std::wstring sourceConfigJson);

// 删除当前镜头中的源
void removeSceneItemFromSource(const char* name);

} // namespace biz

