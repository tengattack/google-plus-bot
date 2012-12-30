
#include "comment.h"

#include "../GooglePlus.h"

namespace gplus {

Comment::Comment(ListValue *lv)
	: has_content(false)
	, m_plusone_count(0)
	, m_is_plusone(false)
	, m_is_mine(false)
	, m_edited(false)
	, m_edited_time(0)
	, m_time(0)
{
	if (lv) parse(lv);
}

Comment::~Comment() {
}

bool Comment::parse(ListValue *lv) {
	if (lv->GetType() == Value::TYPE_LIST) {
		/*
		ID = (string)comment[4];
        if (comment[12].Type == JTokenType.Integer)
            IsMine = true;
		AuthorPhoto = new Uri(((string)comment[16]).Replace("photo.", "s48-c-k/photo."), UriKind.Absolute);
        Author = (string)comment[1];
        AuthorID = (string)comment[6];
		*/
		lv->GetString(4, &id);
		lv->GetString(1, &author);
		lv->GetString(6, &author_id);

		int iMine = 0;
		if (lv->GetInteger(12, &iMine)) {
			m_is_mine = true;
		}

		/*
		DateTime timeStamp;
        if ((double)comment[14] != 0)
        {
            Edited = true;
            timeStamp = Utils.ToDateTime((double)comment[14]);
        }
        else
        {
            Edited = false;
            timeStamp = Utils.ToDateTime((double)comment[3]);
        }
		*/
		m_edited = false;

		double dtime;
		if (lv->GetReal(3, &dtime)) {
			m_time = (uint64)dtime;
		}
		if (lv->GetReal(14, &dtime)) {
			m_edited = true;
			m_edited_time = (uint64)dtime;
		}

		/*
        Content = Utils.ProcessRawContent((string)comment[2], maxContentLength, maxContentLineCount);
        if (IsMine)
            RawContent = (string)comment[5];
		*/
		std::string raw_content;
		if (lv->GetString(2, &raw_content)) {
			has_content = true;
			content = raw_content;
		}

		/*
		PlusOneCount = comment[15][16].Type != JTokenType.Undefined ? (int)comment[15][16] : 0;
        if (PlusOneCount > 0 && (int)comment[15][13] == 1)
            DoIPlusOne = true;
		*/
		//AuthorPhoto = new Uri(((string)comment[16]).Replace("photo.", "s48-c-k/photo."), UriKind.Absolute);

		m_plusone_count = 0;
		m_is_plusone = false;

		ListValue *lvplusone = NULL;
		if (lv->GetList(15, &lvplusone)) {
			lvplusone->GetInteger(16, &m_plusone_count);
			if (m_plusone_count > 0) {
				int iplusone = 0;
				if (lvplusone->GetInteger(13, &iplusone)) {
					if (iplusone) {
						m_is_plusone = true;
					}
				}
			}
		}

		return true;
	}
	return false;
}

bool Comment::PlusOne(GooglePlus *gp, bool set)
{
	if (id.length() > 0) {
		return (gp->PlusOneComment(NULL, id.c_str(), set) == GP_OK);
	}
	return false;
}

}