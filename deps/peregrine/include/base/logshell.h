// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "string"
#include "base/logging.h"
#include "base/strings/sys_string_conversions.h"
#include "base/path_service.h"
#include "base/files/file_path.h"
#include "base/prg.h"
#include "base/asyclogginghandler.h"
#include "base/strings/stringprintf.h"

#include "core/PrgString.h"

#include <iostream>

//#define CONSOLE_OUTPUT

enum LogShellSeverity
{
	lss_info,
	lss_warn,
	lss_error
};

class CLogShell
{
public:
	static void InitLoging(const base::FilePath &settingPath, const base::FilePath &logfilePath)
	{
		base::FilePath logFile = logfilePath.Append(_P("log.log"));
		base::FilePath setFile = settingPath.Append(_P("log.setting"));
		InitAsycLoggingHandler(setFile, logFile);
	}

	CLogShell(const std::string &_category):category(_category)
	{

	}

public:
	// Log
    template<typename CharType>
    void Log(LogShellSeverity level, const CharType* format)
    {
        LogImpl(level, format);
    }

	template<typename CharType, typename A1>
	void Log(LogShellSeverity level, const CharType* format, A1 const& a1)
	{
		LogImpl(level, format, TypeConvert<CharType>(a1));
	}
	
	template<typename CharType, typename A1, typename A2>
	void Log(LogShellSeverity level, const CharType* format, A1 const& a1, A2 const& a2)
	{
		LogImpl(level, format, TypeConvert<CharType>(a1), TypeConvert<CharType>(a2));
	}
	
	template<typename CharType, typename A1, typename A2, typename A3>
	void Log(LogShellSeverity level, const CharType* format, A1 const& a1, A2 const& a2, A3 const& a3)
	{
		LogImpl(level, format, TypeConvert<CharType>(a1), TypeConvert<CharType>(a2), TypeConvert<CharType>(a3));
	}
	
	template<typename CharType, typename A1, typename A2, typename A3, typename A4>
	void Log(LogShellSeverity level, const CharType* format, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4)
	{
		LogImpl(level, format, TypeConvert<CharType>(a1), TypeConvert<CharType>(a2), TypeConvert<CharType>(a3), TypeConvert<CharType>(a4));
	}
	
	template<typename CharType, typename A1, typename A2, typename A3, typename A4, typename A5>
	void Log(LogShellSeverity level, const CharType* format, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5)
	{
		LogImpl(level, format, TypeConvert<CharType>(a1), TypeConvert<CharType>(a2), TypeConvert<CharType>(a3), TypeConvert<CharType>(a4), TypeConvert<CharType>(a5));
	}
	
	template<typename CharType, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	void Log(LogShellSeverity level, const CharType* format, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6)
	{
		LogImpl(level, format, TypeConvert<CharType>(a1), TypeConvert<CharType>(a2), TypeConvert<CharType>(a3), TypeConvert<CharType>(a4), TypeConvert<CharType>(a5), TypeConvert<CharType>(a6));
	}
	
	template<typename CharType, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
	void Log(LogShellSeverity level, const CharType* format, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7)
	{
		LogImpl(level, format, TypeConvert<CharType>(a1), TypeConvert<CharType>(a2), TypeConvert<CharType>(a3), TypeConvert<CharType>(a4), TypeConvert<CharType>(a5), TypeConvert<CharType>(a6), TypeConvert<CharType>(a7));
	}
	
	template<typename CharType, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
	void Log(LogShellSeverity level, const CharType* format, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7, A8 const& a8)
	{
		LogImpl(level, format, TypeConvert<CharType>(a1), TypeConvert<CharType>(a2), TypeConvert<CharType>(a3), TypeConvert<CharType>(a4), TypeConvert<CharType>(a5), TypeConvert<CharType>(a6), TypeConvert<CharType>(a7), TypeConvert(a8));
	}
	
private:
	const CLogShell &LogImpl(LogShellSeverity level, const char *format, ...)
	{
        va_list ap;
        va_start(ap, format);
        std::string log = base::StringPrintV(format, ap);
        va_end(ap);

        log = category + " " + log;
        if (level == lss_error)
        {
            LOG(ERROR) << log;
        }
        else if (level == lss_warn)
        {
            LOG(WARNING) << log;
        }
        else
        {
            LOG(INFO) << log;
        }

#ifdef CONSOLE_OUTPUT
        std::cout << log.c_str() << std::endl;
#endif

        return *this;
	}

    const CLogShell &LogImpl(LogShellSeverity level, const wchar_t* format, ...)
    {
        va_list ap;
        va_start(ap, format);
        std::wstring log = base::StringPrintV(format, ap);
        va_end(ap);

        std::string utf8Log = base::SysWideToUTF8(log);
        utf8Log = category + (" ") + utf8Log;
        if (level == lss_error)
        {
            LOG(ERROR) << utf8Log;
        }
        else if (level == lss_warn)
        {
            LOG(WARNING) << utf8Log;
        }
        else
        {
            LOG(INFO) << utf8Log;
        }

#ifdef CONSOLE_OUTPUT
        std::cout << base::SysWideToNativeMB(log).c_str() << std::endl;
#endif
        
        return *this;
    }
	
	// Convert type to primitive types which are supported by base::StringPrintV
	// Support: integral(include char), float, enum
	template<typename CharType, typename T>
	static typename srlz::internal::enable_if_c< std::is_enum<T>::value || std::is_integral<T>::value || std::is_floating_point<T>::value, T >::type TypeConvert( T const& t )
	{
		return t;
	}
	
	// const CharType*
	template<typename CharType>
	static const CharType* TypeConvert( const CharType* t )
	{
		return t;
	}
	
	template<typename CharType>
	static const CharType* TypeConvert( std::basic_string<CharType> const& t )
	{
		return t.c_str();
	}
	
	template<typename CharType>
	static const CharType* TypeConvert(prg::PrgStringT<CharType> const& t)
	{
		return t.GetString();
	}

public:
	std::string category;
};

#if defined(OS_WIN)
#define __FUNCTION_NAME__ __FUNCTIONW__
#else
#define __FUNCTION_NAME__ __FUNCTION__
#endif

#define WRITELOG(log, LogPriority, LogFmt, ...)	\
    do {std::wstring format = base::StringPrintf(L"%ls, %d, %ls", __FUNCTION_NAME__, __LINE__, LogFmt);\
    log.Log(LogPriority, format.c_str(), __VA_ARGS__);}  while(false)

#define LOG_ERR(log, LogFmt, ...)               WRITELOG(log, lss_error, LogFmt, __VA_ARGS__)
#define LOG_WARNING(log, LogFmt, ...)           WRITELOG(log, lss_warn,  LogFmt, __VA_ARGS__)
#define LOG_INFO(log, LogFmt, ...)              WRITELOG(log, lss_info,  LogFmt, __VA_ARGS__)
#define LOG_ERR_ASSERT(log, LogFmt, ...)        do { WRITELOG(log, lss_error, LogFmt, __VA_ARGS__); assert(false); } while (false)

#define WRITELOG_0(log, LogPriority, LogFmt)	WRITELOG(log, LogPriority, LogFmt, 0)

#define LOG_ERR_0(log, LogFmt)                  WRITELOG_0(log, lss_error, LogFmt)
#define LOG_WARNING_0(log, LogFmt)              WRITELOG_0(log, lss_warn,  LogFmt)
#define LOG_INFO_0(log, LogFmt)                 WRITELOG_0(log, lss_info,  LogFmt)
#define LOG_ERR_ASSERT_0(log, LogFmt)           do { WRITELOG_0(log, lss_error, LogFmt); assert(false); } while (false)
