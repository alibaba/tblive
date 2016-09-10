#pragma once
#include "build/build_config.h"
#include "base/base_export.h"

#if defined(OS_WIN)
#define _P(x) L ## x
#else
#define _P(x) x
#endif

#if defined(OS_WIN)
#include "windows.h"
#endif

#include "base/hresult_define.h"

#if defined(OS_WIN)
#define Prg_API_Export __declspec(dllexport)
#define Prg_API_Import __declspec(dllimport)
#else
#define Prg_API_Export __attribute__((visibility("default")))
#define Prg_API_Import
#endif

