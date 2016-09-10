
#include <Windows.h>
#include <tchar.h>
#include <string>

#include "tblive_cmd_def.h"

extern "C" 
int WINAPI _tWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, 
                                LPTSTR lpCmdLine, int /*nShowCmd*/)
{
	const wchar_t * strCmd = lpCmdLine;

	HWND hwnd = ::FindWindowEx(HWND_MESSAGE, NULL, tblive_cmd_class_name, NULL);
	if (NULL != hwnd)
	{// tblive exists
		COPYDATASTRUCT tmpData;
		ZeroMemory(&tmpData, sizeof(tmpData));
		tmpData.dwData = EXTERNAL_CMD;							// function identifier
		tmpData.cbData = (DWORD)(_tcslen(strCmd) + 1)*sizeof(TCHAR);		// size of data
		tmpData.lpData = (PVOID)strCmd;							// data structure 			
		if (0 == ::SendMessageTimeout(hwnd, WM_COPYDATA, NULL, reinterpret_cast<LPARAM>(&tmpData), SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG, 1000, NULL))
		{
			DWORD err = ::GetLastError();
		}
	}
	else
	{
		// tblive process not exist
		// start a new tblive process with commandline argument
		wchar_t exePath[256] = { 0 };
		::GetModuleFileName(NULL, exePath, 255);
		std::wstring filePath(exePath);
		size_t pos = filePath.rfind(L'\\');
		if ( pos != std::wstring::npos )
		{
			std::wstring fileDir = filePath.substr(0, pos);
			filePath = fileDir + L"\\tblive.exe";
			
			filePath += L" --tblive-cmd=";
			filePath += strCmd;

			{
				wchar_t * fullPath = new wchar_t[filePath.size() + 1];
				::wmemset(fullPath, 0, filePath.size() + 1);
				::wmemcpy_s(fullPath, filePath.size(), filePath.c_str(), filePath.size());
				STARTUPINFO si;
				PROCESS_INFORMATION pi;

				ZeroMemory(&si, sizeof(si));
				ZeroMemory(&pi, sizeof(pi));
				::CreateProcess(
					NULL,
					fullPath,
					NULL,
					NULL,
					false,
					0,
					NULL,
					fileDir.c_str(),
					&si,
					&pi
					);

				delete []fullPath;
			}
		}
	}

	return 0;
}
