
#ifndef _GOOGLE_PLUS_TYPES_NOTIFICATION_H_
#define _GOOGLE_PLUS_TYPES_NOTIFICATION_H_ 1
#pragma once

#include <windows.h>
#include "post.h"

class ListValue;

namespace gplus {

enum NotificationType {
	kNTUnknow,
	kNTPost,
	kNTCircle,
	kNTCount
};

enum NotificationTypeDo {
	kNTDUnknow,
	kNTDComment,
	kNTDPlusOnePost,
	kNTDPlusOneComment,
	kNTDMention,
	kNTDReshare,
	kNTDCount
};

class Notification {
public:
	Notification(ListValue *lv, bool unread);
	~Notification();

	bool parse(ListValue *lv);

	void clear();

	inline bool IsUnread() {
		return m_unread;
	}

	inline LPCSTR GetID() {
		return m_id.c_str();
	}

	inline Post* GetPost() {
		return m_post;
	}

	inline NotificationType GetType() {
		return m_type;
	}

	inline NotificationTypeDo GetTypeDo() {
		return m_type_do;
	}

	std::string m_id;
	std::string m_author_id;
	std::string m_author;
	std::string m_content;

protected:

	Post *m_post;

	bool m_unread;
	NotificationType m_type;
	NotificationTypeDo m_type_do;
};

}

#endif