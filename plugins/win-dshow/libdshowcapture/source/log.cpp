/*
 *  Copyright (C) 2014 Hugh Bailey <obs.jim@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 */

#include "dshow-base.hpp"
#include "log.hpp"
#include "../dshowcapture.hpp"

namespace DShow {

void               *logParam   = NULL;
static LogCallback logCallback = NULL;

void SetLogCallback(LogCallback callback, void *param)
{
	logCallback = callback;
	logParam    = param;
}

static void Log(LogType type, const wchar_t *format, va_list args)
{
	wchar_t str[4096];
	vswprintf_s(str, 4096, format, args);

	if (logCallback)
		logCallback(type, str, logParam);
}

void Error  (const wchar_t *format, ...)
{
	va_list args;
	va_start(args, format);
	Log(LogType::Error, format, args);
	va_end(args);
}

void Warning(const wchar_t *format, ...)
{
	va_list args;
	va_start(args, format);
	Log(LogType::Warning, format, args);
	va_end(args);
}

void Info   (const wchar_t *format, ...)
{
	va_list args;
	va_start(args, format);
	Log(LogType::Info, format, args);
	va_end(args);
}

void Debug  (const wchar_t *format, ...)
{
	va_list args;
	va_start(args, format);
	Log(LogType::Debug, format, args);
	va_end(args);
}

void ErrorHR  (const wchar_t *str, HRESULT hr)
{
	Error(L"%s (0x%08lX): %s", str, hr, ConvertHRToEnglish(hr).c_str());
}

void WarningHR(const wchar_t *str, HRESULT hr)
{
	Warning(L"%s (0x%08lX): %s", str, hr, ConvertHRToEnglish(hr).c_str());
}

void InfoHR   (const wchar_t *str, HRESULT hr)
{
	Info(L"%s (0x%08lX): %s", str, hr, ConvertHRToEnglish(hr).c_str());
}

void DebugHR  (const wchar_t *str, HRESULT hr)
{
	Info(L"%s (0x%08lX): %s", str, hr, ConvertHRToEnglish(hr).c_str());
}

}; /* namespace DShow */
