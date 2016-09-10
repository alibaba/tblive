project(tblive)

option(ENABLE_UI "Enables the OBS user interfaces" ON)
if(DISABLE_UI)
	message(STATUS "UI disabled")
	return()
elseif(ENABLE_UI)
	set(FIND_MODE REQUIRED)
else()
	set(FIND_MODE QUIET)
endif()

if(DEFINED QTDIR${_lib_suffix})
	list(APPEND CMAKE_PREFIX_PATH "${QTDIR${_lib_suffix}}")
elseif(DEFINED QTDIR)
	list(APPEND CMAKE_PREFIX_PATH "${QTDIR}")
elseif(DEFINED ENV{QTDIR${_lib_suffix}})
	list(APPEND CMAKE_PREFIX_PATH "$ENV{QTDIR${_lib_suffix}}")
elseif(DEFINED ENV{QTDIR})
	list(APPEND CMAKE_PREFIX_PATH "$ENV{QTDIR}")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
set(CMAKE_AUTOMOC TRUE)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(_lib_suffix 64)
else()
	set(_lib_suffix 32)
endif()

find_package(Qt5Widgets ${FIND_MODE})
find_package(Qt5WebEngineWidgets ${FIND_MODE})
find_package(Qt5WebChannel ${FIND_MODE})

if(NOT Qt5Widgets_FOUND)
	if (ENABLE_UI)
		message(FATAL_ERROR "Failed to find Qt5")
	else()
		message(STATUS "Qt5 not found - UI disabled")
		return()
	endif()
endif()

include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/libobs")

find_package(Libcurl REQUIRED)
include_directories(${LIBCURL_INCLUDE_DIRS})
add_definitions(${LIBCURL_DEFINITIONS})

add_definitions(-DDISABLE_OBS_UI)

include_directories("${CMAKE_SOURCE_DIR}/deps/peregrine/include") 

include_directories("${CMAKE_SOURCE_DIR}/deps/AuthSDK/AliAuthSDK/pack/include") 

if(WIN32)
	link_directories("${CMAKE_SOURCE_DIR}/deps/peregrine/Win32/lib/$(Configuration)")
	link_directories("${CMAKE_SOURCE_DIR}/deps/AuthSDK/AliAuthSDK/pack/lib/win32/$(Configuration)")
elseif(APPLE)
	link_directories("${CMAKE_SOURCE_DIR}/deps/peregrine/Mac64/bin/${CMAKE_BUILD_TYPE}")
endif()

set(PRG_LIBRARIES 
	base
	net)

if(WIN32)
	set(PRG_LIBRARIES ${PRG_LIBRARIES} ipc)
endif()

set(AUTHSDK_LIBRARIES UnifiedAuthSDK)

if(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4100 /wd4310 /wd4512")
endif()

if(WIN32)
	set(obs_PLATFORM_SOURCES
		platform-windows.cpp
		obs.rc)
elseif(APPLE)
	set(obs_PLATFORM_SOURCES
		platform-osx.mm)

	find_package(AppKit REQUIRED)
	set(obs_PLATFORM_LIBRARIES ${APPKIT_LIBRARIES})

	add_definitions(-fobjc-arc)

	option(ENABLE_SPARKLE_UPDATER "Enables updates via the Sparkle framework (don't forget to update the Info.plist for your .app)" OFF)
	if(ENABLE_SPARKLE_UPDATER)
		find_library(SPARKLE Sparkle)
		include_directories(${SPARKLE})
		set(obs_PLATFORM_SOURCES
			${obs_PLATFORM_SOURCES}
			sparkle-updater.mm)
		set(obs_PLATFORM_LIBRARIES
			${obs_PLATFORM_LIBRARIES}
			${SPARKLE})
		add_definitions(-DUPDATE_SPARKLE=1)
	endif()

elseif(UNIX)
        find_package(Qt5X11Extras REQUIRED)

	set(obs_PLATFORM_SOURCES
		platform-x11.cpp)

	find_package(XCB COMPONENTS XCB REQUIRED RANDR REQUIRED XINERAMA REQUIRED)
	
	include_directories(
		${XCB_INCLUDE_DIRS}
		${X11_XCB_INCLUDE_DIRS})
		
	add_definitions(
		${XCB_DEFINITIONS}
		${X11_XCB_DEFINITIONS})

	set(obs_PLATFORM_LIBRARIES
		${XCB_LIBRARIES}
		${X11_XCB_LIBRARIES}
                Qt5::X11Extras)
endif()

set(obs_SOURCES
	${obs_PLATFORM_SOURCES}
	obs-app.cpp
	window-basic-main.cpp
	window-basic-filters.cpp
	window-basic-settings.cpp
	window-basic-interaction.cpp
	window-basic-properties.cpp
	window-basic-main-outputs.cpp
	window-basic-source-select.cpp
	window-basic-main-scene-collections.cpp
	window-basic-main-transitions.cpp
	window-basic-main-profiles.cpp
	window-basic-main-tblive.cpp
	window-license-agreement.cpp
	window-basic-status-bar.cpp
	window-basic-adv-audio.cpp
	window-basic-transform.cpp
	window-basic-preview.cpp
	window-namedialog.cpp
	window-log-reply.cpp
	window-projector.cpp
	window-remux.cpp
	properties-view.cpp
	focus-list.cpp
	menu-button.cpp
	double-slider.cpp
	volume-control.cpp
	adv-audio-control.cpp
	item-widget-helpers.cpp
	visibility-checkbox.cpp
	vertical-scroll-area.cpp
	visibility-item-widget.cpp
	slider-absoluteset-style.cpp
	source-list-widget.cpp
	qt-display.cpp
	crash-report.cpp
	hotkey-edit.cpp
	source-label.cpp
	remote-text.cpp
	audio-encoders.cpp
	qt-wrappers.cpp
	window-checkcodedialog.cpp
	window-need2ndauthdialog.cpp
	window-loginwnd.cpp
	window-hoverwidget.cpp
	task_pool.cpp
	switches.cpp
	volume-control-custom.cpp)

set(obs_HEADERS
	obs-app.hpp
	platform.hpp
	window-main.hpp
	window-basic-main.hpp
	window-basic-filters.hpp
	window-basic-settings.hpp
	window-basic-interaction.hpp
	window-basic-properties.hpp
	window-basic-main-outputs.hpp
	window-basic-source-select.hpp
	window-license-agreement.hpp
	window-basic-status-bar.hpp
	window-basic-adv-audio.hpp
	window-basic-transform.hpp
	window-basic-preview.hpp
	window-namedialog.hpp
	window-log-reply.hpp
	window-projector.hpp
	window-remux.hpp
	properties-view.hpp
	properties-view.moc.hpp
	display-helpers.hpp
	double-slider.hpp
	focus-list.hpp
	menu-button.hpp
	mute-checkbox.hpp
	volume-control.hpp
	adv-audio-control.hpp
	item-widget-helpers.hpp
	visibility-checkbox.hpp
	vertical-scroll-area.hpp
	visibility-item-widget.hpp
	slider-absoluteset-style.hpp
	source-list-widget.hpp
	qt-display.hpp
	crash-report.hpp
	hotkey-edit.hpp
	source-label.hpp
	remote-text.hpp
	audio-encoders.hpp
	qt-wrappers.hpp
	window-checkcodedialog.hpp
	window-need2ndauthdialog.hpp
	window-loginwnd.hpp
	window-hoverwidget.hpp
	task_pool.hpp
	service_data.h
	switches.h
	volume-control-custom.hpp)

set(obs_UI
	forms/NameDialog.ui
	forms/OBSLicenseAgreement.ui
	forms/OBSLogReply.ui
	forms/OBSBasic.ui
	forms/OBSBasicTransform.ui
	forms/OBSBasicFilters.ui
	forms/OBSBasicSettings.ui
	forms/OBSBasicSourceSelect.ui
	forms/OBSBasicInteraction.ui
	forms/OBSRemux.ui
	forms/CheckCodeDialog.ui
	forms/Need2ndAuthDialog.ui
	forms/HoverWidget.ui
	forms/LoginWnd.ui)

set(obs_QRC
	forms/obs.qrc)

set( tblive_sdk_SOURCES
	TBLiveWebView.hpp
	TBLiveWebView.cpp
	tblive_sdk/SDKTblive.h
	tblive_sdk/SDKTblive.cpp
	tblive_sdk/scene_config.h
	tblive_sdk/tblive_sdk.cc
	tblive_sdk/tblive_sdk.h
	tblive_sdk/biz.cpp
	tblive_sdk/biz.h
)
source_group(tblive_sdk FILES ${tblive_sdk_SOURCES})  

set( login_SOURCES
	login/HostApplication.h
	login/HostApplication.cpp
	login/LoginBiz.h
	login/LoginBiz.cpp
	login/RecentLoginList.h
	login/RecentLoginList.cpp
)
source_group(login FILES ${login_SOURCES})  

set( shell_SOURCES
	shell/tblive_cmd_dispatcher.h
	shell/tblive_cmd_dispatcher.cpp
)
source_group(shell FILES ${shell_SOURCES})  

qt5_wrap_ui(obs_UI_HEADERS ${obs_UI})
qt5_add_resources(obs_QRC_SOURCES ${obs_QRC})

add_executable(tblive WIN32
	${obs_SOURCES}
	${obs_HEADERS}
	${obs_UI_HEADERS}
	${obs_QRC_SOURCES}
	${tblive_sdk_SOURCES}
	${login_SOURCES}
	${shell_SOURCES})

if(WIN32)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(_output_suffix "64")
	else()
		set(_output_suffix "32")
	endif()

	set_target_properties(tblive
		PROPERTIES
			OUTPUT_NAME "tblive")
endif()

target_link_libraries(tblive
	libobs
	libff
	Qt5::Widgets
	Qt5::WebEngineWidgets
	Qt5::WebChannel
	${LIBCURL_LIBRARIES}
	${PRG_LIBRARIES}
	${AUTHSDK_LIBRARIES}
	${obs_PLATFORM_LIBRARIES})

define_graphic_modules(tblive)

install_obs_core(tblive)
install_obs_data(tblive data obs-studio)

if(APPLE)
  install_prg_dylibs()		
endif()

if (UNIX AND UNIX_STRUCTURE AND NOT APPLE)
	install(FILES dist/obs.desktop DESTINATION share/applications)
	install(FILES forms/images/obs.png
		DESTINATION share/icons/hicolor/256x256/apps)
endif()
