#pragma once


#include <base/memory/ref_ptrcounted.h>
#include <base/memory/singleton.h>
#include <base/callback.h>
#include "core/PrgString.h"
#include "base/file_util.h"
#include "base/threading/thread.h"

#include <map>
#include <list>

/*
Format:
{
	"users" : [
		{
			"user" : "zhenshan",
			"auto_token" : "1234567890"
		},
		...
	]
}
*/

struct RecentUserItem
{
	RecentUserItem() : save_psd(false), auto_login(false) {}

	PrgString user;
	PrgString auto_token;
	bool save_psd;
	bool auto_login;
};
EX_SL_CLASS_MAPPING_BEGIN(RecentUserItem)
EX_SL_MEMBER_V(user);
EX_SL_MEMBER_V(auto_token);
EX_SL_MEMBER_V(save_psd);
EX_SL_MEMBER_V(auto_login);
EX_SL_CLASS_MAPPING_END()

struct RecentData
{
	std::list<RecentUserItem> users;
};
EX_SL_CLASS_MAPPING_BEGIN(RecentData)
EX_SL_MEMBER_V(users);
EX_SL_CLASS_MAPPING_END()

class CRecentLoginList
{
public:
	static CRecentLoginList* GetInstance();

public:
	CRecentLoginList();

	void GetRecentUserList(base::Callback<void(std::list<RecentUserItem>)> callback);
	RecentUserItem GetUserItem(PrgString user);

	void AddUser(RecentUserItem userItem);
	void RemoveUser( PrgString user );

private:
	void LoadData(std::list<RecentUserItem> & userMap);
	void SaveData(std::list<RecentUserItem> const& userMap);

private:
	friend struct StaticMemorySingletonTraits < CRecentLoginList >;
	DISALLOW_COPY_AND_ASSIGN(CRecentLoginList);

	std::string m_desKey;
	base::Thread m_thread;

	bool m_localDataInited;
	std::list<RecentUserItem> m_recentUsers;
};
