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

#pragma once

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

namespace DShow {

void Error  (const wchar_t *format, ...);
void Warning(const wchar_t *format, ...);
void Info   (const wchar_t *format, ...);
void Debug  (const wchar_t *format, ...);

void ErrorHR  (const wchar_t *str, HRESULT hr);
void WarningHR(const wchar_t *str, HRESULT hr);
void InfoHR   (const wchar_t *str, HRESULT hr);
void DebugHR  (const wchar_t *str, HRESULT hr);

}; /* namespace DShow */
