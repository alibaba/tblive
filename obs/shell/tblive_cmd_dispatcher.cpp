
#include "tblive_cmd_dispatcher.h"

#include "../../shell/tblive_cmd_def.h"
#include "tblive_sdk/biz.h"
#include "switches.h"

#include <Windows.h>
#include <map>

#include "url/gurl.h"
#include "core/PrgString.h"
#include "net/base/escape.h"
#include "base/basictypes.h"
#include "base/command_line.h"

namespace {

void ParseParams(const PrgString& str, std::map<PrgString, PrgString>& params)
{
	PrgString strParam,strKey,strValue;
	int pos = str.IsEmpty()? -1 : 0;
	int subpos = 0;
	while(pos >=0)
	{
		strParam = str.Tokenize(_T("&"), pos);
		if(!strParam.IsEmpty())
		{
			subpos = strParam.Find(_T("="));
			strKey = strParam.Left(subpos).Trim();
			if(params.find(strKey) != params.end()) continue;
			if(subpos > 0) 
			{
				strValue = strParam.Right(strParam.GetLength()-subpos-1);

				// Url unescape
				std::string utf8Val = base::WideToUTF8(strValue.GetString());
				utf8Val = net::UnescapeURLComponent(utf8Val, net::UnescapeRule::URL_SPECIAL_CHARS);
				strValue = base::UTF8ToWide(utf8Val);
			}
			else 
				strValue.Empty();

			params[strKey] = strValue.Trim();
		}
	}
}

void DispatchTbliveCmdImpl(PrgString cmd, std::map<PrgString, PrgString> const& args)
{
	std::map<std::wstring, std::wstring> wstrArgs;
	for ( auto item : args )
	{
		wstrArgs[item.first.GetString()] = item.second.GetString();
	}
	biz::dispatchTBLiveCmd(cmd.GetString(), wstrArgs);
}

void ParseTbliveCmd(PrgString strCmd, PrgString& cmd, std::map<PrgString, PrgString>& args)
{
	cmd.Empty();
	args.clear();

	// Scheme: tblive://cmd?biz=taobaojiaoyu&url=xxx
	const wchar_t * TBLIVE_SCHEME = L"tblive://";

	// check scheme
	strCmd.TrimLeft(L"\"");
	strCmd.TrimRight(L"\"");
	//strCmd.Trim(L'"');
	if (strCmd.Find(TBLIVE_SCHEME) != 0)
	{
		return;
	}

	const int SCHEME_LEN = wcslen(TBLIVE_SCHEME);
	int pos = strCmd.Find(L"?");
	if (pos > SCHEME_LEN)
	{
		cmd = strCmd.Mid(SCHEME_LEN, pos - SCHEME_LEN);
		cmd.Trim(L'/');

		ParseParams(strCmd.Mid(pos + 1), args);
	}
	else
	{
		cmd = strCmd.Mid(SCHEME_LEN);
	}
}

void DispatchTbliveCmd(PrgString strCmd)
{
	PrgString cmd;
	std::map<PrgString, PrgString> args;
	ParseTbliveCmd(strCmd, cmd, args);

	DispatchTbliveCmdImpl(cmd, args);
}

LRESULT CALLBACK DispWindowProc(HWND hwnd,
							UINT uMsg,
							WPARAM wParam,
							LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COPYDATA:
		{
			COPYDATASTRUCT* tmpData = reinterpret_cast<COPYDATASTRUCT*>(lParam);
			if (tmpData->dwData != EXTERNAL_CMD || tmpData->lpData == NULL) 
				break;

			PrgString strCmd(reinterpret_cast<LPCTSTR>(tmpData->lpData), tmpData->cbData / sizeof(TCHAR));
			DispatchTbliveCmd(strCmd);
			return 0;
		}
		break;
	default:
	    break;
	}

	return DefWindowProc(hwnd,uMsg,wParam,lParam);
}

}// namespace


void InitTbliveCmd()
{
	// Init window
	WNDCLASSEX	wndclass;

	memset(&wndclass,0,sizeof(wndclass));
	wndclass.cbSize = sizeof(wndclass);
	wndclass.lpfnWndProc  = DispWindowProc ;
	wndclass.hInstance = ::GetModuleHandle(NULL);
	wndclass.lpszClassName = tblive_cmd_class_name;

	if (!RegisterClassEx (&wndclass))
	{
		return;
	}

	HWND m_hWnd = ::CreateWindow(tblive_cmd_class_name, _T(""), 0, 0, 0, 0, 0,
		HWND_MESSAGE, (HMENU)NULL, NULL, NULL);

	if(NULL == m_hWnd)
	{
		assert(false && "tblive cmd window init failed.");
	}

	// Init preset cmd
	CommandLine * current = CommandLine::ForCurrentProcess();
	if (current)
	{
		std::wstring strCmd = current->GetSwitchValueNative(switches::kTbliveCmd);
		if (!strCmd.empty())
		{
			PrgString cmd;
			std::map<PrgString, PrgString> args;
			ParseTbliveCmd(strCmd, cmd, args);
			if (cmd == switches::cmdStartLive)
			{
				std::map<std::wstring, std::wstring> cmd_args;
				for ( auto item : args )
				{
					cmd_args[item.first.GetString()] = item.second.GetString();
				}
				CTBLiveShell::GetInstance()->SetCmds( cmd_args );
			}
		}
	}
}


// CTBLiveShell
CTBLiveShell::CTBLiveShell()
{}

CTBLiveShell::~CTBLiveShell()
{}

/*static*/ CTBLiveShell* CTBLiveShell::GetInstance()
{
	return Singleton < CTBLiveShell,
		StaticMemorySingletonTraits<CTBLiveShell> > ::get();
}

void CTBLiveShell::SetCmds(std::map<std::wstring, std::wstring> cmds)
{
	m_cmds = cmds;
}

std::wstring CTBLiveShell::GetCmd(std::wstring key)
{
	std::wstring val;
	auto fit = m_cmds.find(key);
	if ( fit != m_cmds.end() )
	{
		val = fit->second;
	}
	return val;
}

std::map<std::wstring, std::wstring> CTBLiveShell::GetCmds()
{
	return m_cmds;
}

bool CTBLiveShell::IsEmpty()
{
	return m_cmds.empty();
}
