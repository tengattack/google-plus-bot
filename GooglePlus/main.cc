
#include <stdio.h>
#include <locale.h>
#include <time.h>
#include <tcmalloc.h>

#include <map>

#include <net/net.h>

#include <base/string/string_number_conversions.h>

#include <base/file/file.h>
#include <base/file/filedata.h>

#include <common/strconv.h>
#include <common/Buffer.h>
#include <common/wiseint.h>

#include <engine/script_engine.h>

#include <v8.h>

#include "GooglePlus.h"
#include "gplustypes/post.h"
#include "gplustypes/notifications.h"

#include "rule.h"
#include "cache.h"
#include "config.h"
#include "gplus_script.h"

time_t startup_lasttime = 0;
std::string gp_startup_cookie;
CScriptGooglePlus gp;

CACHE_TABLE posts_cache;
//CACHE_TABLE comments_cache;

enum RefreshModeType {
	kRMTNotice = 0,
	kRMTStream,
	kRMTCommunityStream,
	kRMTCount
};

void RefreshCache()
{
	ClearOldCache(&posts_cache, CACHE_MAX_TIME);
	//ClearOldCache(&comments_cache, CACHE_MAX_TIME);
}

bool BeforeExit()
{
	printf("Saving state...\n");

	if (gp_startup_cookie != gp.GetCookie()) {
		//DeleteFileW(config::cookiepath.c_str());

		base::CFile file;
		if (file.Open(base::kFileCreate, config::cookiepath.c_str())) {
			file.Write((PBYTE)gp.GetCookie(), lstrlenA(gp.GetCookie()));
			file.Close();
		}
	}

	if (config::imode == (int)kRMTCommunityStream && startup_lasttime != gp.GetCommunityLasttime()) {

		std::string strlasttime = base::Int64ToString(gp.GetCommunityLasttime());

		//DeleteFileW(config::lasttimepath.c_str());
		base::CFile file;
		if (file.Open(base::kFileCreate, config::lasttimepath.c_str())) {
			file.Write((PBYTE)strlasttime.c_str(), strlasttime.length());
			file.Close();
		}
	}

	net::Uninit();
	return true;
}

BOOL WINAPI ConsoleHandler(DWORD e)
{
	switch(e)
	{
	case CTRL_C_EVENT:
		/*MessageBoxA(NULL,
			"CTRL+C received!","CEvent",MB_OK);
		break;*/
	case CTRL_BREAK_EVENT:
		/*MessageBoxA(NULL,
			"CTRL+BREAK received!","CEvent",MB_OK);
		break;*/
	case CTRL_CLOSE_EVENT:
		/*MessageBoxA(NULL,
			"Program being closed!","CEvent",MB_OK);
		break;*/
	case CTRL_LOGOFF_EVENT:
		/*MessageBoxA(NULL,
			"User is logging off!","CEvent",MB_OK);
		break;*/
	case CTRL_SHUTDOWN_EVENT:
		/*MessageBoxA(NULL,
			"User is logging off!","CEvent",MB_OK);
		break;*/

		BeforeExit();
		ExitProcess(0);
		break;
	}

	return TRUE;
}

int wmain(int argc, wchar_t *argv[])
{
	//(PHANDLER_ROUTINE)
	if (SetConsoleCtrlHandler(ConsoleHandler, TRUE) == FALSE) {
		// unable to install handler... 
		// display message to the user
		printf("Unable to install handler!\n");
		//return -1;
	}

	setlocale(LC_ALL, "chs");

	if (!net::Init()) {
		printf("net::Init failed!\n");
		return 1;
	}

	config::CheckUpdate();

	if (!config::LoadConfig((argc > 1) ? argv[1] : NULL)) {
		printf("LoadConfig failed!\n");
		return 1;
	}

	if (config::imode >= kRMTCount || config::imode < 0) {
		printf("Error Mode: %d!\n", config::imode);
		return 1;
	}

	base::CFile file;
	if (file.Open(base::kFileRead, config::cookiepath.c_str())) {
		base::CFileData fd;
		if (fd.Read(file)) {
			std::string cookie;
			fd.ToText(cookie);
			gp.SetCookie(cookie.c_str());
			gp.RefreshCookie();
		}
		file.Close();
	} else if (config::email.length() > 0 && config::password.c_str() > 0) {
		printf("Logining...\n");
		if (gp.Login(config::email.c_str(), config::password.c_str()) != GP_OK) {
			printf("Login failed!\n");
			return 1;
		}

		DeleteFileW(config::cookiepath.c_str());

		CBuffer buf;
		buf.Write((PBYTE)gp.GetCookie(), lstrlenA(gp.GetCookie()));
		buf.FileWrite(config::cookiepath.c_str());
	} else {
		printf("Need login!\n");
		return 1;
	}

	//remove email & password from memory
	config::email.clear();
	config::password.clear();

	if (file.Open(base::kFileRead, config::lasttimepath.c_str())) {
		base::CFileData fd;
		if (fd.Read(file)) {
			std::string strlasttime;
			fd.ToText(strlasttime);

			int64 t = 0;
			if (base::StringToInt64(strlasttime, &t)) {
				if (t > 0) gp.SetCommunityLasttime(t);
			}
		}
		file.Close();
	}
	
	//cache state
	gp_startup_cookie = gp.GetCookie();
	startup_lasttime = gp.GetCommunityLasttime();

	SetGlobalGooglePlus(&gp);

	tae.Init(config::jsfolderpath.c_str(), config::script_base_path.c_str());

	printf("GetBaseInfo...\n");
	gp.GetBaseInfo();

	printf("UserID: %s\n", gp.GetUserID());

	if (config::community_id.length() > 0) {
		printf("CommunityID: %s\n", config::community_id.c_str());
		gp.SetCommunityID(config::community_id.c_str());
	}

	if (config::page_id.length() > 0) {
		printf("PageID: %s\n", config::page_id.c_str());
		gp.SetPage(config::page_name.c_str(), config::page_id.c_str());
	} else {
		gp.SetPage(config::username.c_str());
	}
	printf("\n");

	RefreshModeType rmtype = (RefreshModeType)config::imode;

	if (rmtype == kRMTCommunityStream && config::community_id.length() <= 0) {
		printf("No CommunityID!\n");
		return 1;
	}
	
	if (config::initscript.length() > 0) {
		printf("Run init script...\n");
		tae.RunEx(config::initscript.c_str(), NULL, NULL, true);
	}

	ListValue *lv = NULL;

	while (true) {

		switch (rmtype) {
		case kRMTNotice:
			if (gp.GetNotificationsData(&lv, true) == GP_OK) {

				gplus::Notifications nc(lv);
				//printf("Unread: %d\n", nc.GetUnreadCount());

				for (int i = 0; i < nc.GetCount(); i++) {
					gplus::Notification *notice = nc.GetNotification(i);
					if (notice && notice->IsUnread() && notice->GetType() == gplus::kNTPost && 
							(notice->GetTypeDo() == gplus::kNTDMention || notice->GetTypeDo() == gplus::kNTDComment)) {

						RuleProcessContent(&gp, 
							notice->m_author, notice->m_author_id, notice->m_content, notice->m_id, 
							notice->GetPost());
					}
				}
			} else {
				printf("GetNotificationsData failed!\n");
			}
			break;
		case kRMTStream:
			if (gp.GetActivities(&lv) == GP_OK) {
				std::vector<gplus::Post *> newposts;
				std::vector<gplus::Post *> posts;
				ListValue::iterator iter = lv->begin();
				while (iter != lv->end()) {
					gplus::Post *p = new gplus::Post((ListValue *)*iter);
					posts.push_back(p);
					iter++;
				}
				delete lv;
				lv = NULL;

				int i;
				bool init_cache = (posts_cache.size() <= 0);
				for (i = 0; i < posts.size(); i++) {
					CACHE *ppc = &posts_cache[posts[i]->id];

					ppc->t = time(NULL);
					if ((int)ppc->wi == 0) {
						newposts.push_back(posts[i]);
					} else {
						delete posts[i];
					}
					ppc->wi++;
				}

				if (!init_cache) {
					RefreshCache();
					for (i = 0; i < newposts.size(); i++) {
						RuleProcessContent(&gp, 
							newposts[i]->author, newposts[i]->author_id, newposts[i]->content, newposts[i]->id, 
							newposts[i]);
						delete newposts[i];
					}
				}
			} else {
				printf("GetActivities failed!\n");
			}
			break;
		case kRMTCommunityStream:
			if (gp.GetCommunities(&lv) == GP_OK) {
				std::vector<gplus::Post *> posts;
				ListValue::iterator iter = lv->begin();
				while (iter != lv->end()) {
					gplus::Post *p = new gplus::Post((ListValue *)*iter);
					posts.push_back(p);
					iter++;
				}
				delete lv;
				lv = NULL;

				for (int i = 0; i < posts.size(); i++) {
					RuleProcessContent(&gp, 
						posts[i]->author, posts[i]->author_id, posts[i]->content, posts[i]->id, 
						posts[i]);
					delete posts[i];
				}
			} else {
				printf("GetCommunities failed!\n");
			}
			break;
		}
		if (lv) {
			delete lv;
			lv = NULL;
		}
		Sleep(REFRESH_TIME);
	}

	BeforeExit();
	return 0;
}