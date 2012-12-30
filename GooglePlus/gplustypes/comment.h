
#ifndef _GOOGLE_PLUS_TYPES_COMMENT_H_
#define _GOOGLE_PLUS_TYPES_COMMENT_H_ 1
#pragma once

#include <string>
#include <base/string/values.h>

class GooglePlus;

namespace gplus {

class Comment {
public:
	Comment(ListValue *lv = NULL);
	~Comment();

	bool parse(ListValue *lv);

	bool PlusOne(GooglePlus *gp, bool set);

	std::string id;
	std::string author_id;
	std::string author;
	std::string content;

	int m_plusone_count;
	bool m_is_plusone;

	int m_is_mine;

	bool m_edited;
	uint64 m_edited_time;
	uint64 m_time;

	bool has_content;
};

}

#endif