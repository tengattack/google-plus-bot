
#ifndef _TA_GOOGLE_PLUS_RULE_H_
#define _TA_GOOGLE_PLUS_RULE_H_ 1
#pragma once

#include <string>
#include <vector>

class DictionaryValue;

class GooglePlus;

namespace gplus {
	class Post;
}

enum MatchMode {
	kPosContent = 0,
	kPosAuthor,
	kRegexContent,
	kRegexAuthor,
	kFullContent,
	kFullAuthor,
	kSelfPosContent,
	kSelfRegexContent,
	kSelfFullContent,
	kPosUserPosContent,
	kPosUserRegexContent,
	kPosUserFullContent,
	kRegexUserPosContent,
	kRegexUserRegexContent,
	kRegexUserFullContent,
	kFullUserPosContent,
	kFullUserRegexContent,
	kFullUserFullContent,
	kMMCount
};

typedef struct _MATCH {
	int mode;
	std::vector<std::string> match;
	//std::vector<std::string> reply;
} MATCH;

typedef struct _REPLY_MATCH {
	MATCH m;
	std::vector<std::string> reply;
} REPLY_MATCH;

typedef struct _SCRIPT_MATCH {
	MATCH m;
	//bool use_file;
	std::wstring script;
} SCRIPT_MATCH;

typedef std::vector<MATCH> RULE_TABLE;
typedef std::vector<REPLY_MATCH> REPLY_RULE_TABLE;
typedef std::vector<SCRIPT_MATCH> SCRIPT_RULE_TABLE;

int LoadRuleTable(DictionaryValue *dv);

void RuleProcessContent(GooglePlus *gp, 
						std::string& author, std::string& authorId, std::string& content, std::string& postId,
						gplus::Post *post);

#endif