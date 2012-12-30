
#include "rule.h"

#include <boost/regex.hpp>

#include <base/string/values.h>
#include <base/json/json_writer.h>
#include <base/json/json_reader.h>

#include <base/file/file.h>
#include <base/file/filedata.h>

#include <common/strconv.h>

#include "GooglePlus.h"
#include "gplustypes/post.h"
#include "gplus_script.h"

REPLY_RULE_TABLE reply_table;
RULE_TABLE ignore_table, plusone_table;
SCRIPT_RULE_TABLE script_table;

int LoadCommonRuleTable(ListValue *lv, RULE_TABLE *pm)
{
	int count = 0;
	if (lv) {
		ListValue::iterator iter = lv->begin();
		while (iter != lv->end()) {
			DictionaryValue *dv = (DictionaryValue *)(*iter);
			if (dv && dv->GetType() == Value::TYPE_DICTIONARY) {
				ListValue *match = NULL;
				MATCH m;

				m.mode = 0;
				dv->GetInteger("mode", &m.mode);
				if (dv->GetList("match", &match)) {
					ListValue::iterator iter_match = match->begin();
					while (iter_match != match->end()) {
						std::string t;
						if ((*iter_match)->GetAsString(&t)) {
							m.match.push_back(t);
						}
						iter_match++;
					}
				}

				bool addmatch = false;
				if (m.mode < kMMCount && m.match.size() > 0) {
					//双重判定
					if (m.mode > kSelfFullContent) {
						if (m.match.size() == 2) {
							addmatch = true;
						}
					} else {
						addmatch = true;
					}
				}/* else if (rm.m.mode < kRMMCount && rm.m.mode >= kIgnorePosUser && rm.m.match.size() > 0) {
					addmatch = true;
				}*/

				if (addmatch) {
					pm->push_back(m);
					count++;
				}
			}
			iter++;
		}
	}	//if (lv) {
	return count;
}

int LoadReplyRuleTable(ListValue *lv, REPLY_RULE_TABLE *prm)
{
	int count = 0;
	if (lv) {
		ListValue::iterator iter = lv->begin();
		while (iter != lv->end()) {

			DictionaryValue *dv = (DictionaryValue *)(*iter);
			if (dv && dv->GetType() == Value::TYPE_DICTIONARY) {
				ListValue *match = NULL, *reply = NULL;
				REPLY_MATCH rm;

				rm.m.mode = 0;
				dv->GetInteger("mode", &rm.m.mode);
				if (dv->GetList("match", &match)) {
					ListValue::iterator iter_match = match->begin();
					while (iter_match != match->end()) {
						std::string t;
						if ((*iter_match)->GetAsString(&t)) {
							rm.m.match.push_back(t);
						}
						iter_match++;
					}
				}
				if (dv->GetList("reply", &reply)) {
					ListValue::iterator iter_reply = reply->begin();
					while (iter_reply != reply->end()) {
						std::string t;
						if ((*iter_reply)->GetAsString(&t)) {
							rm.reply.push_back(t);
						}
						iter_reply++;
					}
				}
							
				bool addmatch = false;
				if (rm.m.mode < kMMCount && rm.m.match.size() > 0 && rm.reply.size() > 0) {
					//双重判定
					if (rm.m.mode > kSelfFullContent) {
						if (rm.m.match.size() == 2) {
							addmatch = true;
						}
					} else {
						addmatch = true;
					}
				}/* else if (rm.m.mode < kRMMCount && rm.m.mode >= kIgnorePosUser && rm.m.match.size() > 0) {
					addmatch = true;
				}*/

				if (addmatch) {
					reply_table.push_back(rm);
					count++;
				}
			}	//if (dv && dv->GetType()
			iter++;
		}
	}	//if (lv
	return count;
}

int LoadScriptRuleTable(ListValue *lv, SCRIPT_RULE_TABLE *prm)
{
	int count = 0;
	if (lv) {
		ListValue::iterator iter = lv->begin();
		while (iter != lv->end()) {

			DictionaryValue *dv = (DictionaryValue *)(*iter);
			if (dv && dv->GetType() == Value::TYPE_DICTIONARY) {
				ListValue *match = NULL;
				SCRIPT_MATCH sm;

				sm.m.mode = 0;
				dv->GetInteger("mode", &sm.m.mode);
				if (dv->GetList("match", &match)) {
					ListValue::iterator iter_match = match->begin();
					while (iter_match != match->end()) {
						std::string t;
						if ((*iter_match)->GetAsString(&t)) {
							sm.m.match.push_back(t);
						}
						iter_match++;
					}
				}

				//默认使用文件
				/*sm.use_file = true;
				dv->GetBoolean("usefile", &sm.use_file);*/

				dv->GetString("script", &sm.script);
							
				bool addmatch = false;
				if (sm.m.mode < kMMCount && sm.m.match.size() > 0 && sm.script.length() > 0) {
					addmatch = true;
				}

				if (addmatch) {
					prm->push_back(sm);
					count++;
				}
			}	//if (dv && dv->GetType()
			iter++;
		}
	}	//if (lv
	return count;
}

int LoadRuleTable(DictionaryValue *dv)
{
	int count = 0;

	if (dv->GetType() == Value::TYPE_DICTIONARY) {
		ListValue *lv = NULL;

		static struct _TableMap {
			char name[32];
			RULE_TABLE *match_table;
		} tm[] = {
			{"ignore", &ignore_table},
			{"plusone", &plusone_table},
		};
		for (int ii = 0; ii < sizeof(tm) / sizeof(tm[0]); ii++) {
			dv->GetList(tm[ii].name, &lv);
			count += LoadCommonRuleTable(lv, tm[ii].match_table);
		}

		lv = NULL;
		dv->GetList("reply", &lv);
		count += LoadReplyRuleTable(lv, &reply_table);

		lv = NULL;
		dv->GetList("script", &lv);
		count += LoadScriptRuleTable(lv, &script_table);
	} //if (dv->GetType()

	return count;
}

bool IsMatch(MATCH *pm, bool bself, std::string& author, std::string& content, std::string *regex_replace_text = NULL, std::string *regex_replace_outtext = NULL)
{
	bool is_match = false;
	int j;
	
	for (j = 0; j < pm->match.size(); j++) {

		try {

		boost::regex reg(pm->match[j]);

		if (bself) {
			switch ((MatchMode)pm->mode) {
			case kSelfPosContent:
				if (strstr(content.c_str(), pm->match[j].c_str())) {
					is_match = true;
				}
				break;
			case kSelfRegexContent:
				if (boost::regex_match(content, reg)) {
					if (regex_replace_text && regex_replace_outtext) {
						*regex_replace_outtext = boost::regex_replace(content, reg, regex_replace_text->c_str());
					}
					is_match = true;
				}
				break;
			case kSelfFullContent:
				if (content == pm->match[j]) {
					is_match = true;
				}
				break;
			}
		} else {
			bool bdoublecheck = false;
			switch ((MatchMode)pm->mode) {
			case kPosContent:
				if (strstr(content.c_str(), pm->match[j].c_str())) {
					is_match = true;
				}
				break;
			case kPosAuthor:
				if (strstr(author.c_str(), pm->match[j].c_str())) {
					is_match = true;
				}
				break;
			case kRegexContent:
				if (boost::regex_match(content, reg)) {
					if (regex_replace_text && regex_replace_outtext) {
						*regex_replace_outtext = boost::regex_replace(content, reg, regex_replace_text->c_str());
					}
					is_match = true;
				}
				break;
			case kRegexAuthor:
				if (boost::regex_match(author, reg)) {
					if (regex_replace_text && regex_replace_outtext) {
						*regex_replace_outtext = boost::regex_replace(author, reg, regex_replace_text->c_str());
					}
					is_match = true;
				}
				break;
			case kFullContent:
				if (content == pm->match[j]) {
					is_match = true;
				}
				break;
			case kFullAuthor:
				if (author == pm->match[j]) {
					is_match = true;
				}
				break;
			case kPosUserPosContent:
			case kPosUserRegexContent:
			case kPosUserFullContent:
				if (j == 0 && strstr(author.c_str(), pm->match[j].c_str())) {
					bdoublecheck = true;
				}
				break;
			case kRegexUserPosContent:
			case kRegexUserRegexContent:
			case kRegexUserFullContent:
				//bdoublecheck = true;
				if (j == 0 && boost::regex_match(author, reg)) {
					/*if (regex_replace_text && regex_replace_outtext) {
						*regex_replace_outtext = boost::regex_replace(author, reg, regex_replace_text->c_str());
					}*/
					bdoublecheck = true;
				}
				break;
			case kFullUserPosContent:
			case kFullUserRegexContent:
			case kFullUserFullContent:
				if (j == 0 && author == pm->match[j]) {
					bdoublecheck = true;
				}
				break;
			}
			if (bdoublecheck && j == 0 && pm->match.size() > 1) {

				boost::regex reg2(pm->match[j + 1]);

				switch ((MatchMode)pm->mode) {
				case kPosUserPosContent:
				case kRegexUserPosContent:
				case kFullUserPosContent:
					if (strstr(content.c_str(), pm->match[j + 1].c_str())) {
						is_match = true;
					}
					break;
				case kPosUserRegexContent:
				case kRegexUserRegexContent:
				case kFullUserRegexContent:
					if (boost::regex_match(content, reg2)) {
						if (regex_replace_text && regex_replace_outtext) {
							*regex_replace_outtext = boost::regex_replace(content, reg2, regex_replace_text->c_str());
						}
						is_match = true;
					}
					break;
				case kPosUserFullContent:
				case kRegexUserFullContent:
				case kFullUserFullContent:
					if (content == pm->match[j + 1]) {
						is_match = true;
					}
					break;
				}
			}
		}
		if (is_match) {
			break;
		}

		} catch (...) {
			wchar_t *wmatch = NULL;
			lo_Utf82W(&wmatch, pm->match[j].c_str());
			wprintf(L"regex: %s exception!\n", wmatch);
			free(wmatch);
		}
	}

	return is_match;
}

void RuleProcessContent(GooglePlus *gp, 
						std::string& author, std::string& authorId, std::string& content, std::string& postId,
						gplus::Post *post)
{
	wchar_t *wauthor = NULL, *wcontent = NULL;

	lo_Utf82W(&wauthor, author.c_str());
	lo_Utf82W(&wcontent, content.c_str());
	wprintf(L"%s: %s\n", wauthor, wcontent);
	free(wauthor);
	free(wcontent);

	bool bself = (authorId == gp->GetUserID());
	bool bplusone = false;
	bool bignore = false;
	int i, j;
	for (i = 0; i < ignore_table.size(); i++) {
		if (IsMatch(&ignore_table[i], bself, author, content)) {
			bignore = true;
			break;
		}
	}
	if (bignore) {
		wprintf(L"ignore this.\n\n");
		return;
	}

	for (i = 0; i < script_table.size(); i++) {
		if (IsMatch(&script_table[i].m, bself, author, content)) {
			//使用脚本

			UserParam *up = new UserParam;
			up->gp = gp;
			up->post = post;
			up->user = gp->GetUser();
			up->user_id = gp->GetUserID();
			up->author = author;
			up->author_id = authorId;
			up->content = content;
			up->post_id = postId;

			up->e = CreateEvent(NULL, FALSE, FALSE, NULL);

			HANDLE hThread = tae.RunEx(script_table[i].script.c_str(), ScriptCallback, up, false);
			if (hThread) {
				ResumeThread(hThread);
				CloseHandle(hThread);

				WaitForSingleObject(up->e, INFINITE);
				CloseHandle(up->e);
				up->e = NULL;
			} else {
				delete up;
				printf("CreateThread failed!\n");
			}

			break;
		}
	}

	for (i = 0; i < plusone_table.size(); i++) {
		if (IsMatch(&plusone_table[i], bself, author, content)) {
			bplusone = true;
			break;
		}
	}

	uint32 tick = GetTickCount();
	std::string regex_text;
	LPCSTR reply_text = NULL;
	for (i = 0; i < reply_table.size(); i++) {
		std::vector<std::string> *pTmpReplyList = &reply_table[i].reply;
		int iReply = tick % pTmpReplyList->size();
		if (IsMatch(&reply_table[i].m, bself, author, content, &((*pTmpReplyList)[iReply]), &regex_text)) {
			if (regex_text.length() > 0) {
				reply_text = regex_text.c_str();
			} else {
				reply_text = (*pTmpReplyList)[iReply].c_str();
			}
			break;
		}
	}

	if (reply_text) {
		wchar_t *wreply = NULL;
		lo_Utf82W(&wreply, reply_text);
		wprintf(L"reply: %s\n", wreply);
		free(wreply);
		gp->Comment(postId.c_str(), reply_text);
	}

	if (bplusone) {
		wprintf(L"plusone\n");
		gp->PlusOnePost(postId.c_str(), true);
	}

	wprintf(L"\n");
}
