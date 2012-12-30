
#ifndef _GOOGLE_PLUS_TYPES_NOTIFICATIONS_H_
#define _GOOGLE_PLUS_TYPES_NOTIFICATIONS_H_ 1
#pragma once

#include <vector>
#include "notification.h"

class ListValue;

namespace gplus {

class Notifications {
public:
	Notifications(ListValue *lv = NULL);
	~Notifications();

	bool parse(ListValue *lv);

	void clear();

	inline int GetUnreadCount() {
		return m_unread_count;
	}

	inline int GetCount() {
		return m_notifications.size();
	}

	inline Notification* GetNotification(int index) {
		if (index >= 0 && index < m_notifications.size()) {
			return m_notifications[index];
		}
		return NULL;
	}

protected:

	int m_unread_count;
	std::vector<Notification *> m_notifications;
};

}

#endif