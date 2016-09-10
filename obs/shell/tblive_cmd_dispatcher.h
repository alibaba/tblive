#pragma once

#include "core/PrgString.h"
#include "base/memory/singleton.h"


extern void InitTbliveCmd();


// CTBLiveShell
class CTBLiveShell
{
public:
	CTBLiveShell();
	~CTBLiveShell();

	static CTBLiveShell* GetInstance();

	void SetCmds(std::map<std::wstring, std::wstring> cmds);
	std::wstring GetCmd(std::wstring key);
	std::map<std::wstring, std::wstring> GetCmds();

	bool IsEmpty();

private:
	friend struct StaticMemorySingletonTraits < CTBLiveShell >;
	DISALLOW_COPY_AND_ASSIGN(CTBLiveShell);

	std::map<std::wstring, std::wstring> m_cmds;
};
