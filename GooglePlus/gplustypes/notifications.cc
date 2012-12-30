
#include "notifications.h"

#include <base/string/values.h>

namespace gplus {

Notifications::Notifications(ListValue *lv)
	: m_unread_count(0)
{
	if (lv) parse(lv);
}

Notifications::~Notifications()
{
	clear();
}

void Notifications::clear()
{
	m_unread_count = 0;

	for (int i = 0; i < m_notifications.size(); i++) {
		delete m_notifications[i];
	}

	m_notifications.clear();
}

bool Notifications::parse(ListValue *lv)
{
	clear();

	int unreadCount = 0;
	int count = 0;
	//(int)array[7];
	if (lv->GetInteger(7, &unreadCount)) {
		m_unread_count = unreadCount;

		ListValue *sublv = NULL;
		if (lv->GetList(0, &sublv)) {
			/*for (int i = 0; i < ((JArray)array[0]).Count; i++)
			{
				JToken notification = array[0][i];
				Notification n = new Notification(notification, i < unreadCount);
				((MainViewModel)DataContext).Notifications.Add(n);
			}*/
			int i = 0;
			ListValue::iterator iter = sublv->begin();

			while (iter != sublv->end()) {
				if ((*iter)->GetType() == Value::TYPE_LIST) {
					Notification *notice = new Notification((ListValue *)*iter, count < unreadCount);
					m_notifications.push_back(notice);
				} else {
					m_notifications.push_back(NULL);
				}
				count++;
				iter++;
			}
		}
	}

	return (count > 0);
}

}