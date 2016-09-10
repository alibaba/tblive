
#include "RecentLoginList.h"

#include "base/file_util.h"
#include "net/http/des.h"
#include "net/base/file_stream.h"
#include "HostApplication.h"
#include "base/json/json_helper.h"
#include "base/base_util.h"


/*static*/
CRecentLoginList* CRecentLoginList::GetInstance()
{
	return Singleton < CRecentLoginList,
		StaticMemorySingletonTraits<CRecentLoginList> > ::get();
}

CRecentLoginList::CRecentLoginList()
	: m_thread("tblive_login")
	, m_localDataInited(false)
{
	// magic key
	m_desKey = "2202663F";

	m_thread.Start();
}

void CRecentLoginList::GetRecentUserList(base::Callback<void(std::list<RecentUserItem>)> callback)
{
	if (m_localDataInited)
	{
		callback.Run(m_recentUsers);
		return;
	}

	m_thread.message_loop()->PostTask(FROM_HERE, base::Lambda([this,callback](){
		std::list<RecentUserItem> userList;
		LoadData(userList);

		base::GetUIMessageLoop()->PostTask(FROM_HERE, base::Lambda([this, userList, callback](){
			bool hasLogined = false;
			if ( !m_recentUsers.empty() )
			{// rare happen
				hasLogined = true;
			}

			for ( auto it : userList )
			{
				m_recentUsers.push_back(it);
			}

			if ( !callback.is_null() )
			{
				callback.Run(m_recentUsers);
			}
			m_localDataInited = true;

			if ( hasLogined )
			{
				m_thread.message_loop()->PostTask(FROM_HERE, base::Lambda([this](){
					SaveData(m_recentUsers);
				}));
			}
		}));
	}));
}

void CRecentLoginList::LoadData(std::list<RecentUserItem> & userList)
{
	userList.clear();

	base::FilePath file = CHostApplication::GetInstance()->GetDataDir();
	file = file.Append(L"global.dat");

	std::string content;
	bool suc = base::ReadFileToString(file, &content);
	if (suc && !content.empty())
	{
		uint8 iv[] = { 1, 2, 3, 4, 5, 6, 7, 8 };

		std::string result;
		bool des_ok = net::DesDecrypt(m_desKey, content, iv, net::KP_CCMODE_CBC, net::KP_CCPADDING_PKCS_PADDING, result);
		if ( des_ok && !result.empty() )
		{
			RecentData data;
			json::JsonStrToClass(result, data);

			userList = data.users;
		}
	}
}

void CRecentLoginList::SaveData(std::list<RecentUserItem> const& userList)
{
	base::FilePath file = CHostApplication::GetInstance()->GetDataDir();
	file = file.Append(L"global.dat");

	RecentData data;
	data.users = userList;
	std::string content = json::JsonStrFromClass(data);

	std::string result;

	uint8 iv[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
	bool res = net::DesEncrypt(m_desKey, content, iv, net::KP_CCMODE_CBC, net::KP_CCPADDING_PKCS_PADDING, result);
	if (res && !result.empty()) {
		file_util::WriteFile(file, result.c_str(), (int)result.size());
	}
}

void CRecentLoginList::AddUser(RecentUserItem userItem)
{
	// remove old if exist
	auto fit = std::find_if(m_recentUsers.begin(), m_recentUsers.end(), [userItem](RecentUserItem item){
		return item.user == userItem.user;
	});

	if ( fit != m_recentUsers.end() )
	{
		m_recentUsers.erase(fit);
	}

	// add new
	m_recentUsers.push_front(userItem);

	if ( m_localDataInited )
	{
		// save
		m_thread.message_loop()->PostTask(FROM_HERE, base::Lambda([this](){
			SaveData(m_recentUsers);
		}));
	}
}

void CRecentLoginList::RemoveUser(PrgString user)
{
}

RecentUserItem CRecentLoginList::GetUserItem(PrgString user)
{
	auto fit = std::find_if(m_recentUsers.begin(), m_recentUsers.end(), [user](RecentUserItem item){
		return item.user == user;
	});

	if (fit != m_recentUsers.end())
	{
		return *fit;
	}

	return RecentUserItem();
}
