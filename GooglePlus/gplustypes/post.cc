
#include "post.h"

#include "../GooglePlus.h"

namespace gplus {

Post::Post(ListValue *lv)
	: original(NULL)
	, has_content(false)
	, is_reshare(false)
	, m_reshare_count(0)
	, m_comment_count(0)
	, m_plusone_count(0)
	, m_is_plusone(false)
	, m_edited(false)
	, m_edited_time(0)
	, m_time(0)
	, m_in_community(false)
{
	if (lv) parse(lv);
}

Post::~Post() {
	clear();
}

void Post::clear()
{
	if (original) {
		delete original;
		original = NULL;
	}

	for (int i = 0; i < m_comments.size(); i++) {
		delete m_comments[i];
	}

	m_comments.clear();

	/*
	id.clear();
	from.clear();
	author.clear();
	author_id.clear();

	content.clear();

	has_content = false;
	is_reshare = false;
	m_reshare_count = 0;
	m_comment_count = 0;
	m_plusone_count = 0;
	m_is_plusone = false;

	m_edited = false;
	m_edited_time = 0;
	m_time = 0;

	m_in_community = false
	m_community_name.clear();
	m_community_category.clear();
	*/
}

bool Post::parse(ListValue *lv) {
	if (lv->GetType() == Value::TYPE_LIST) {
		/*
		ID = (string)post[8];
        AuthorPhoto = new Uri(((string)post[18]).Replace("photo.", "s64-c-k/photo."), UriKind.Absolute);
        Author = (string)post[3];
        AuthorID = (string)post[16];
		*/
		lv->GetString(8, &id);
		lv->GetString(2, &from);
		lv->GetString(3, &author);
		lv->GetString(16, &author_id);

		/*
		DateTime timeStamp;
        if (post[70].Type != JTokenType.Undefined)
        {
            Edited = true;
            timeStamp = Utils.ToDateTime((double)post[70] / 1000);
        }
        else
        {
            Edited = false;
            timeStamp = Utils.ToDateTime((double)post[5]);
        }
		*/
		m_edited = false;

		double dtime;
		if (lv->GetReal(5, &dtime)) {
			m_time = (uint64)dtime;
		}
		if (lv->GetReal(70, &dtime)) {
			m_edited = true;
			m_edited_time = (uint64)dtime;
		}
		/*
		if (post[44].Type != JTokenType.Undefined)
        {
            string rawContent = (string)post[47];
            HasContent = rawContent != "";
            Content = Utils.ProcessRawContent(rawContent, maxContentLength, maxContentLineCount);
            IsReshare = true;
            OriginalAuthorPhoto = new Uri(((string)post[44][4]).Replace("photo.", "s48-c-k/photo."), UriKind.Absolute);
            OriginalAuthor = (string)post[44][0];
            OriginalAuthorID = (string)post[44][1];
            rawContent = (string)post[4];
            HasOriginalContent = rawContent != "";
            OriginalContent = Utils.ProcessRawContent(rawContent, maxContentLength, maxContentLineCount);
        }
        else
        {
            string rawContent = post[47].Type == JTokenType.Undefined ? (string)post[4] : (string)post[47];
            HasContent = rawContent != "";
            Content = Utils.ProcessRawContent(rawContent, maxContentLength, maxContentLineCount);
            IsReshare = false;
        }
		*/
		std::string raw_content;
		ListValue *sublv = NULL;
		if (lv->GetList(44, &sublv)) {
			original = new Post;
			is_reshare = true;

			//sublv->GetString(8, &id);
			sublv->GetString(0, &original->author);
			sublv->GetString(1, &original->author_id);
			if (lv->GetString(47, &raw_content)) {
				has_content = true;
				content = raw_content;
				raw_content.clear();
			}
			if (sublv->GetString(4, &raw_content)) {
				original->has_content = true;
				original->content = raw_content;
			}
		} else {
			is_reshare = false;
			if (lv->GetString(4, &raw_content)) {
				has_content = true;
				content = raw_content;
			}
				
		}

		/*PlusOneCount = (int)post[73][16];
        if (PlusOneCount > 0 && (int)post[73][13] == 1)
            DoIPlusOne = true;
        ReshareCount = ((JArray)post[25]).Count;
        CommentCount = (int)post[93];

        foreach (JToken reply in post[7])
            Replies.Add(new Comment(reply, maxContentLength, maxContentLineCount));*/
		m_plusone_count = 0;
		m_is_plusone = false;

		ListValue *lvplusone = NULL;
		if (lv->GetList(73, &lvplusone)) {
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

		m_reshare_count = 0;
		if (lv->GetList(25, &sublv)) {
			m_reshare_count = sublv->GetSize();
		}

		// Media = new Media(post[97]);
		if (lv->GetList(97, &sublv)) {
			m_media.parse(sublv);
		}

		m_comment_count = 0;
		lv->GetInteger(93, &m_comment_count);

		//只有部分的评论
		if (lv->GetList(7, &sublv)) {
			ListValue::iterator iter = sublv->begin();
			while (iter != sublv->end()) {
				if ((*iter)->GetType() == Value::TYPE_LIST) {
					Comment *comment = new Comment((ListValue *)*iter);
					m_comments.push_back(comment);
				} else {
					m_comments.push_back(NULL);
				}
				iter++;
			}
		}

		//社群分类 108 2
		m_in_community = false;
		if (lv->GetList(108, &sublv)) {
			m_in_community = true;
			sublv->GetString(1, &m_community_name);
			sublv->GetString(2, &m_community_category);
		}

		return true;
	}
	return false;
}

bool Post::PlusOne(GooglePlus *gp, bool set)
{
	if (id.length() > 0) {
		return (gp->PlusOnePost(id.c_str(), set) == GP_OK);
	}
	return false;
}

}