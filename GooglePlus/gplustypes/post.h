
#ifndef _GOOGLE_PLUS_TYPES_POST_H_
#define _GOOGLE_PLUS_TYPES_POST_H_ 1
#pragma once

#include <string>
#include <vector>
#include <base/string/values.h>

#include "comment.h"
#include "media.h"

class GooglePlus;

namespace gplus {

class Post {
public:
	Post(ListValue *lv = NULL);
	~Post();

	bool parse(ListValue *lv);

	bool PlusOne(GooglePlus *gp, bool set);

	void clear();

	int m_reshare_count;
	int m_comment_count;
	int m_plusone_count;
	bool m_is_plusone;

	std::string id;
	std::string author_id;
	std::string author;
	std::string content;
	
	std::string from;

	bool has_content;
	bool is_reshare;

	bool m_edited;
	uint64 m_edited_time;
	uint64 m_time;

	bool m_in_community;
	std::string m_community_name;
	std::string m_community_category;

	Post *original;

	inline Comment* GetComment(int i) {
		if (i >= 0 && i < m_comment_count && i < m_comments.size()) {
			return m_comments[i];
		}
		return NULL;
	}

	Media m_media;

protected:
	std::vector<Comment *> m_comments;
};

}

#endif