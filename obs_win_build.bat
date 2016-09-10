:: check build mode
@set BuildParam=%1
@if not "%BuildParam%"=="Release" set BuildParam="Debug"
@echo "tvlive build mode: " %BuildParam%

:: set debug or release
@set debug_type=1
@if "%BuildParam%"=="Release" set debug_type=0

:: specify the obs_studio src root.
set obs_root=%~dp0%

:: specify the Qt installation path
@if "%QTDIR%"=="" set QTDIR=C:\Qt\Qt5.6.0\5.6\msvc2013\)

:: set vs2013
set vs2013="%VS120COMNTOOLS%..\..\Common7\IDE\devenv.com"

@mkdir build
@cd/d %obs_root%/build

@set build_type="RelWithDebInfo|Win32"
@if {%debug_type%} == {1} (set build_type="Debug|Win32")

@set config_mode="RelWithDebInfo"
@if {%debug_type%} == {1} (set config_mode="Debug")

call cmake -G "Visual Studio 12 2013" -DCMAKE_BUILD_TYPE=%config_mode% ..

:: build with vs2013
@del build_tblive.txt

call %vs2013% %obs_root%/build/tblive.sln /build %build_type% /out build_tblive.txt

::call %vs2013% %obs_root%/build/tblive.sln
@cd/d %obs_root%
