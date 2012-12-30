
#ifndef _TA_GOOGLE_PLUS_H_
#define _TA_GOOGLE_PLUS_H_ 1
#pragma once

#include <string>

#include <base/basictypes.h>
#include <base/string/values.h>

#include <net/http/TACookiesManger.h>

struct curl_slist;
typedef void CURL;

class CBuffer;

namespace base {
	class Properties;
}

enum GP_URL_TYPE {
	kGPHome,
	kGPNewPost,
	kGPGetActivity,
	kGPGetActivities,
	kGPPushInit,
	kGPPushReceive,
	kGPInitImageUpload,
	kGPUploadImage,
	kGPComment,
	kGPPlusOne,
	kGPLinkPreview,
	kGPGetNotificationsData,
	kGPUpdateLastReadTime,
	kGPGUC,
	kGPGetProfile,
	kGPEditComment,
	kGPDeleteComment,
	kGPLookupCircles,
	kGPCommunities,
	kGPUrlCount
};

#define GP_OK					0
#define GP_LOGIN_FAILED			1
#define GP_2STEP_LOGIN_FAILED	2
#define GP_2STEP_NO_TOKEN		3


class GooglePlus : public CTANetBase {
public:
	GooglePlus();
	~GooglePlus();

	int Comment(LPCSTR postId, LPCWSTR commentContent);
	int Comment(LPCSTR postId, LPCSTR utf8commentContent);

	int GetNotificationsData(ListValue **lvNotifications, bool auto_update_last_read_time = true);
	int UpdateLastReadTime(uint64 lasttime);

	int GetActivities(ListValue **lvActivities);
	int GetActivity(LPCSTR postId, ListValue **lvActivity);
	int Login(LPCSTR email, LPCSTR password);

	int PlusOnePost(LPCSTR postId, bool set);
	int PlusOneComment(LPCSTR postId, LPCSTR commentId, bool set);

	int GetCommunities(ListValue **lvCommunities, time_t lasttime = 0);

	static void Content(std::string& content);

	int GetBaseInfo();

	void RefreshCookie();

	inline LPCSTR GetUserID() {
		return user_id.c_str();
	}

	inline LPCSTR GetUser() {
		return m_user.c_str();
	}

	inline time_t GetCommunityLasttime() {
		return m_community_lasttime;
	}

	inline void SetCommunityLasttime(time_t lasttime) {
		m_community_lasttime = lasttime;
	}

	//set pageId to recover
	void SetPage(LPCSTR user, LPCSTR pageId = NULL);
	void SetCommunityID(LPCSTR communityId);

protected:

	int RequestPost(LPCSTR url, base::Properties& pro, CBuffer& response, CBuffer *header = NULL);
	int RequestPost(LPCSTR url, LPCSTR postdata, CBuffer& response, CBuffer *header = NULL);
	int RequestGet(LPCSTR url, CBuffer& response, CBuffer *header = NULL);

	void FormatUrl(std::string& url, GP_URL_TYPE type, void *param = NULL);

	virtual curl_slist* post_addheader(curl_slist *list);
	virtual void post_setopt(CURL *curl);
	virtual curl_slist* addheader(curl_slist *list);
	virtual void setopt(CURL *curl);

	CTACookiesManger m_cookie_mgr;

	std::string session_id;
	std::string user_id;

	bool m_set_page;
	std::string m_page_id;
	std::string m_user;

	std::string m_community_id;
	time_t m_community_lasttime;
};

#endif