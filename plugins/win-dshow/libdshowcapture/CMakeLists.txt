cmake_minimum_required(VERSION 2.8.12)

project(libdshowcapture)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

OPTION(BUILD_SHARED_LIBS "Build shared library" ON)

find_package(CXX11 REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_FLAGS}")

if(${CMAKE_C_COMPILER_ID} MATCHES "Clang" OR ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
	set(CMAKE_COMPILER_IS_CLANG TRUE)
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG)
	set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-function -Werror-implicit-function-declaration -Wno-missing-field-initializers ${CMAKE_CXX_FLAGS} -fno-strict-aliasing")
	set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-unused-function -Werror-implicit-function-declaration -Wno-missing-braces -Wno-missing-field-initializers ${CMAKE_C_FLAGS} -std=gnu99 -fno-strict-aliasing")
	
	option(USE_LIBC++ "Use libc++ instead of libstdc++" ${APPLE})
	if(USE_LIBC++)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
	endif()
elseif(MSVC)
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()

	# Disable pointless constant condition warnings
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4127 /wd4201")
endif()

if(WIN32)
	add_definitions(-DUNICODE -D_UNICODE)
endif()

if(MSVC)
	set(CMAKE_C_FLAGS_DEBUG "/DDEBUG=1 /D_DEBUG=1 ${CMAKE_C_FLAGS_DEBUG}")
	set(CMAKE_CXX_FLAGS_DEBUG "/DDEBUG=1 /D_DEBUG=1 ${CMAKE_C_FLAGS_DEBUG}")

	if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
		set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /SAFESEH:NO")
	endif()
else()
	if(MINGW)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_WIN32_WINNT=0x0600 -DWINVER=0x0600")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0600 -DWINVER=0x0600")
	endif()
	set(CMAKE_C_FLAGS_DEBUG "-DDEBUG=1 -D_DEBUG=1 ${CMAKE_C_FLAGS_DEBUG}")
	set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG=1 -D_DEBUG=1 ${CMAKE_CXX_FLAGS_DEBUG}")
endif()

if(MINGW)
    include (CheckSymbolExists)
    check_symbol_exists(MINGW_HAS_SECURE_API "_mingw.h" HAVE_MINGW_HAS_SECURE_API)
    if(NOT HAVE_MINGW_HAS_SECURE_API)
        message(FATAL_ERROR "mingw must be compiled with --enable-secure-api")
    endif()
endif()

set(libdshowcapture_SOURCES
	source/capture-filter.cpp
	source/output-filter.cpp
	source/dshowcapture.cpp
	source/dshowencode.cpp
	source/device.cpp
	source/encoder.cpp
	source/dshow-base.cpp
	source/dshow-demux.cpp
	source/dshow-enum.cpp
	source/dshow-formats.cpp
	source/dshow-media-type.cpp
	source/dshow-encoded-device.cpp
	source/log.cpp)

set(libdshowcapture_HEADERS
	dshowcapture.hpp
	source/IVideoCaptureFilter.h
	source/capture-filter.hpp
	source/output-filter.hpp
	source/device.hpp
	source/encoder.hpp
	source/dshow-base.hpp
	source/dshow-demux.hpp
	source/dshow-device-defs.hpp
	source/dshow-enum.hpp
	source/dshow-formats.hpp
	source/dshow-media-type.hpp
	source/log.hpp)

add_library(libdshowcapture
	${libdshowcapture_SOURCES}
	${libdshowcapture_HEADERS})

target_link_libraries(libdshowcapture
	strmiids
	ksuser
	wmcodecdspuuid)
