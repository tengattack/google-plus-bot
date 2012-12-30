
#include "GooglePlus.h"
#include "gp_api.h"

#include <time.h>
#include <curlhelper.h>

#include <base/string/string_number_conversions.h>
#include <base/string/stringprintf.h>
#include <base/string/values.h>
#include <base/string/values_op.h>
#include <base/json/json_writer.h>
#include <base/json/json_reader.h>

#include <common/properties.h>
#include <common/Buffer.h>
#include <common/Urlcode.h>
#include <common/strconv.h>

#include <net/net.h>

#define USERAGENT "Mozilla/5.0 (Windows NT 6.2; WOW64) AppleWebKit/537.21 (KHTML, like Gecko) Chrome/25.0.1354.0 Safari/537.21"
//#define USERAGENT "Mozilla/5.0 tengattack-google-plus-client"

GooglePlus::GooglePlus()
	: m_set_page(false)
	, m_community_lasttime(0)
{
	m_cookie_mgr.bind(this);
}


GooglePlus::~GooglePlus()
{

}

curl_slist* GooglePlus::addheader(struct curl_slist *list)
{
	list = curl_slist_append(list, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
	list = curl_slist_append(list, "Accept-Charset: GBK,utf-8;q=0.7,*;q=0.3");
	list = curl_slist_append(list, "Accept-Language: zh-CN,zh;q=0.8");
	list = curl_slist_append(list, "Accept-Encoding: gzip,deflate");
	//list = curl_slist_append(list, "Cache-Control: max-age=0");
	list = curl_slist_append(list, "Connection: keep-alive");

	return CTANetBase::addheader(list);
}

void GooglePlus::setopt(CURL *curl)
{
	CTANetBase::setopt(curl);

	curl_easy_setopt(curl, CURLOPT_USERAGENT, USERAGENT);

	//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);

	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

	//curl_easy_setopt(curl, CURLOPT_RETURNTRANSFER, 1)
}

int GetInput(const char *html, base::Properties& pro)
{
	char *input = (char *)html;
	char *inputend = NULL;
	char *namestart = NULL, *nameend = NULL;
	char *valstart = NULL, *valend = NULL;

	int count = 0;
	
	while (input = strstr(input, "<input ")) {
		input += 7;
		inputend = strstr(input, ">");
		if (inputend) {
			inputend[0] = 0;

			namestart = strstr(input, "name=\"");
			valstart = strstr(input, "value=\"");

			if (valstart) {
				valstart += 7;
				valend = strstr(valstart, "\"");
				if (valend) {
					valend[0] = 0;
				} else {
					valstart = NULL;
				}
			}

			if (namestart) {
				namestart += 6;
				nameend = strstr(namestart, "\"");
				if (nameend) {
					nameend[0] = 0;
					if (valstart) {
						CURLEncodeA urlvalencode(valstart, lstrlenA(valstart));
						CURLEncodeA urlnameencode(namestart, lstrlenA(namestart));
						pro.Put(urlnameencode.c_str(), urlvalencode.c_str());
					} else {
						pro.Put(namestart, "");
					}

					count++;
				}
			}

			
			input = inputend + 1;
		}
	}

	return count;
}

bool IsMovedTemporarily(const char *header)
{
	//走了代理可能会同时出现200和302的情况
	return (strstr(header, "HTTP/1.1 302 ") != NULL);
	//return (strncmp(header, "HTTP/1.1 302 ", 13) == 0);
}

bool IsHttpOK(const char *header)
{
	return (strncmp(header, "HTTP/1.1 200 ", 13) == 0);
}

void GooglePlus::Content(std::string& content)
{
	
}

void GooglePlus::FormatUrl(std::string& url, GP_URL_TYPE type, void *param)
{
	if (m_set_page) {
		std::string page_sub_url;
		base::SStringPrintf(&page_sub_url, "b/%s/", m_page_id.c_str());
		base::SStringPrintf(&url, GP_URL[type], page_sub_url.c_str(), time(NULL));
	} else {
		base::SStringPrintf(&url, GP_URL[type], "", time(NULL));
	}
}

int GooglePlus::GetBaseInfo()
{
	int ret = -1;
	CBuffer htmlbuffer;
	CBuffer header;
	if ((ret = RequestGet(GP_URL[kGPHome], htmlbuffer, &header)) == GP_OK) {
		m_cookie_mgr.explain((char *)header.GetBuffer());
	}

	const char *html = (const char *)htmlbuffer.GetBuffer();
	char *sid = strstr((char *)html, "AObGSA");
	char *uid = strstr((char *)html, "oid=\"");		//第一个总是自己的用户ID
	if (sid) {
		char *sidend = strstr(sid, "\"");
		if (sidend) {
			sidend[0] = 0;
			session_id = sid;
		}
	}
	if (uid) {
		uid += 5;
		char *uidend = strstr(uid, "\"");
		if (uidend) {
			uidend[0] = 0;
			user_id = uid;
		}
	}
	return ret;
}

void GooglePlus::SetPage(LPCSTR user, LPCSTR pageId)
{
	if (pageId && lstrlenA(pageId) > 0) {
		m_set_page = true;;
		m_page_id = pageId;
	} else {
		m_set_page = false;
		m_page_id.clear();
	}

	m_user = user ? user : "";
}

void GooglePlus::SetCommunityID(LPCSTR communityId)
{
	if (communityId && lstrlenA(communityId) > 0) {
		m_community_id = communityId;
	} else {
		m_community_id.clear();
	}
}

void GooglePlus::RefreshCookie()
{
	m_cookie_mgr.refresh();
}

int GooglePlus::Login(LPCSTR email, LPCSTR password)
{
	int ret = -1;
	CBuffer htmlbuffer;
	CBuffer header;
	if ((ret = RequestGet("https://accounts.google.com/ServiceLogin?hl=en&continue=http://www.google.com/", htmlbuffer, &header)) != GP_OK) {
		return ret;
	}

	ret = -2;

	//cookie
	m_cookie_mgr.explain((char *)header.GetBuffer());

	const char *html = (const char *)htmlbuffer.GetBuffer();

	/*
	int index = result.RawData.IndexOf("id=\"dsh\"") + 16;
    string dsh = result.RawData.Substring(index, result.RawData.IndexOf("\"", index) - index);
    index = result.RawData.IndexOf("name=\"GALX\"", index) + 28;
    string galx = result.RawData.Substring(index, result.RawData.IndexOf("\"", index) - index);
	*/
	const char *dshstart = strstr(html, "id=\"dsh\"");

	if (dshstart) {
		dshstart += 16;

		const char *GALXstart = strstr(dshstart, "name=\"GALX\"");

		if (GALXstart) {
			GALXstart += 28;

			char *dshend = strstr((char *)dshstart, "\"");
			char *GALXend = strstr((char *)GALXstart, "\"");

			if (dshend && GALXend) {
				dshend[0] = 0;
				GALXend[0] = 0;

				/*string payload = "dsh=" + dsh +
                        "&GALX=" + galx +
                        "&Email=" + email +
                        "&Passwd=" + password +
                        "&PersistentCookie=yes&signIn=Sign in";*/

				base::Properties pro;
				pro.Put("dsh", dshstart);
				pro.Put("GALX", GALXstart);
				pro.Put("Email", email);
				pro.Put("Passwd", password);
				pro.Put("PersistentCookie", "yes");
				pro.Put("signIn", "Sign%20in");

				/*if (GetInput((const char *)htmlbuffer.GetBuffer(), pro) <= 0) {
					return ret;
				}*/

				htmlbuffer.ClearBuffer();
				header.ClearBuffer();

				if ((ret = RequestPost("https://accounts.google.com/ServiceLoginAuth", pro, htmlbuffer, &header)) == GP_OK) {

					ret = GP_LOGIN_FAILED;

					//跳转了，成功了
					if (IsMovedTemporarily((const char *)header.GetBuffer())) {

						char *h = (char *)header.GetBuffer();

						char *location = strstr(h, "Location:");

						//不map的话会修改值，故先查找location
						m_cookie_mgr.explain(h);

						if (location) {
							location += 9;
							if (location[0] == ' ') location++;

							char *locationend = strstr(location, "\r\n");
							if (locationend) locationend[0] = 0;

							//二步验证
							if (strstr(location, "SmsAuth") != NULL) {

								std::string urllocaltion = location;

								htmlbuffer.ClearBuffer();
								header.ClearBuffer();

								if ((ret = RequestGet(urllocaltion.c_str(), htmlbuffer, &header)) != GP_OK) {
									return ret;
								}

								m_cookie_mgr.explain((char *)header.GetBuffer());

								ret = GP_2STEP_LOGIN_FAILED;

								
								char *tokenstart = strstr((char *)htmlbuffer.GetBuffer(), "name=\"smsToken\"");
								char *tokenend = NULL;
								if (tokenstart) {
									tokenstart += 17;
									tokenstart = strstr(tokenstart, "value=\"");
									if (tokenstart) {
										tokenstart += 7;

										tokenend = strstr(tokenstart, "\"");
										if (tokenend) tokenend[0] = 0;
									}
								}

								if (!tokenend) {
									return ret;
								}

								if (lstrlenA(tokenstart) <= 0) {
									ret = GP_2STEP_NO_TOKEN;
									return ret;
								}

								base::Properties prosms;

								//存在2个<form>
								/*
								if (GetInput((const char *)htmlbuffer.GetBuffer(), prosms) <= 0) {
									return ret;
								}

								std::string token;
								prosms.Get("smsToken", token);

								if (token.length() <= 0) {
									return GP_2STEP_NO_TOKEN;
								}

								prosms.Put("smsUserPin", smspin);
								*/

								char smspin[64] = {0};
								printf("2-step Auth: ");
								scanf("%s", smspin);

								prosms.Put("timeStmp", "");
								prosms.Put("secTok", "");
								prosms.Put("smsToken", tokenstart);
								prosms.Put("smsUserPin", smspin);
								prosms.Put("smsVerifyPin", "Verify");
								prosms.Put("PersistentCookie", "yes");
								
								htmlbuffer.ClearBuffer();
								header.ClearBuffer();

								if ((ret = RequestPost(urllocaltion.c_str(), prosms, htmlbuffer, &header)) != GP_OK) {
									return ret;
								}

								ret = GP_2STEP_LOGIN_FAILED;

								m_cookie_mgr.explain((char *)header.GetBuffer());

								base::Properties jmppro;
								if (GetInput((const char *)htmlbuffer.GetBuffer(), jmppro) <= 0) {
									return ret;
								}

								htmlbuffer.ClearBuffer();
								header.ClearBuffer();

								if ((ret = RequestPost("https://accounts.google.com/ServiceLoginAuth", jmppro, htmlbuffer, &header)) != GP_OK) {
									return ret;
								}

								m_cookie_mgr.explain((char *)header.GetBuffer());

								if (IsMovedTemporarily((const char *)header.GetBuffer())) {
									ret = GP_OK;
								}

							} else {
								ret = GP_OK;
							}
						}
					}
				}
			}
		}
	}


	return ret;
}

//loadPost
int GooglePlus::GetActivity(LPCSTR postId, ListValue **lvActivity)
{
	int ret = -1;
	//std::string postId = "z13mddaqyun5e11uq23cc5rj1muuwf42d";

	//obj = new JArray(postId, null, null, null, 5);

	ListValue lv;
	ListValueAppend(&lv, 5,
		Value::CreateStringValue(postId),
		NULL, NULL, NULL,
		Value::CreateIntegerValue(5)
	);

	std::string json;
	base::JSONWriter::Write(&lv, false, &json);

	CURLEncodeA urldata(json.c_str(), json.length());

	base::Properties pro;
	pro.Put("f.req", urldata.c_str());
	pro.Put("at", session_id.c_str());

	CBuffer htmlbuffer;
	CBuffer header;

	std::string url;
	FormatUrl(url, kGPGetActivity);

	if ((ret = RequestPost(url.c_str(), pro, htmlbuffer, &header)) != GP_OK) {
		return ret;
	}

	ret = -2;

	//cookie
	m_cookie_mgr.explain((char *)header.GetBuffer());

	if (!IsHttpOK((const char *)header.GetBuffer())) {
		return ret;
	}
	
	const char *res = (const char *)htmlbuffer.GetBuffer();
	if (strncmp(res, ")]}'\n\n", 6) == 0) res += 6;

	ListValue *list = (ListValue *)base::JSONReader::Read(res, true);
	
	if (list) {
		if (list->GetType() == Value::TYPE_LIST) {
			//DataContext = new Post(JArray.Parse(result.RawData.Substring(6))[0][1][1]);
			ListValue *tl = NULL;
			bool bget = false;
			list->GetList(0, &tl);
			if (tl) {
				tl->GetList(1, &tl);
				if (tl) {
					if (tl->GetList(1, NULL)) {
						tl->Remove(1, (Value **)lvActivity);
						bget = true;
					}
				}
			}

			if (bget) {
				ret = GP_OK;
			}
		}
		delete list;
	}

	return ret;
}


int GooglePlus::GetCommunities(ListValue **lvCommunities, time_t lasttime)
{
	int ret = -1;
	time_t t;

	if (lasttime) {
		t = lasttime;
		m_community_lasttime = t;
	} else if (m_community_lasttime) {
		t = m_community_lasttime;
	} else {
		t = time(NULL) * 1000;
		m_community_lasttime = t;
	}

	/*
		f.req:["111681436473928069449",null,["z12iix4axqqefpnn223hcdczykuuehoik04","z13osnhapqaucnbgr04cct4i5paoipuyrbs","z13ptdmg1ny0s5wlw23iipsxwqmjwhxbb04","z12gyv0rcza2irrok04cd3hh4rfst5jped4","z13thpsqzlv0yd45g04chp153zu3fddqlu00k","z13ddzopdy3kd3fqg04ccb2grzvty5kh3qs0k","z12ahx2jbq32hzlxn04cd5c4plfhuf1zjlo0k","z12vybh4dqnwcjadl23iipsxwqmjwhxbb04","z12iirsyimijhn2iy04ch11rbmzfghponrk0k","z13iezqomovchxjke22kvbdygzfyhd01t","z13fij2alzrqw5smn23hcdczykuuehoik04","z124wzer5sq5hvylp23iipsxwqmjwhxbb04","z12iybvasq2rd3b5n04chp153zu3fddqlu00k","z13bzvjp5xrxxj2ne23cchtwruv3hh4xw04","z132xxsobwbxiplae04chjpgzrryjnvzpqk0k","z13jexvzdqrecxdv4223h1wwulz1vbwcr","z13ecteywrfjd111v235y3aypnfnwfdp404","z13rhnm43reeslno204chtur3rmudzlroww0k","z13izb5ahyjsv30g304cetto2ymngrbaozw0k","z12nwzy52qewxvunz23cc5rj1muuwf42d"],1356836630992]
		at:AObGSAhI5AmXvPgyV5AUeoQdUbMlZcXB3w:1356834212203
	*/

	ListValue lv;
	ListValueAppend(&lv, 4,
		Value::CreateStringValue(m_community_id),
		NULL, NULL,
		Value::CreateRealValue(t)
	);

	std::string json;
	base::JSONWriter::Write(&lv, false, &json);

	CURLEncodeA urldata(json.c_str(), json.length());

	base::Properties pro;
	pro.Put("f.req", urldata.c_str());
	pro.Put("at", session_id.c_str());

	CBuffer htmlbuffer;
	CBuffer header;

	std::string url;
	FormatUrl(url, kGPCommunities);

	if ((ret = RequestPost(url.c_str(), pro, htmlbuffer, &header)) != GP_OK) {
		return ret;
	}

	ret = -2;

	//cookie
	m_cookie_mgr.explain((char *)header.GetBuffer());

	if (!IsHttpOK((const char *)header.GetBuffer())) {
		return ret;
	}

	const char *res = (const char *)htmlbuffer.GetBuffer();
	if (strncmp(res, ")]}'\n\n", 6) == 0) res += 6;

	ListValue *list = (ListValue *)base::JSONReader::Read(res, true);
	
	if (list) {
		if (list->GetType() == Value::TYPE_LIST) {
			//list = (JArray)result.ResponseObject[0][1][1][0];
			ListValue *tl = NULL;
			bool bget = false;
			list->GetList(0, &tl);
			if (tl && tl->GetList(2, &tl)) {
				if (tl) {
					double ltime = 0;
					if (tl->GetReal(3, &ltime)) {
						m_community_lasttime = ltime;
					}
					if (tl->GetList(1, NULL)) {
						tl->Remove(1, (Value **)lvCommunities);
						bget = true;
					}
				}
			}

			if (bget) {
				ret = GP_OK;
			}
		}
		delete list;
	}

	return ret;
}

//loadStream
int GooglePlus::GetActivities(ListValue **lvActivities)
{
	int ret = -1;
	/*
	array[1] = array[0] = 1;
            array[7] = "social.google.com";
            array[15] = array[8] = new JArray();
            array[26] = new JArray((object)new JArray(5, 3, 4, 1));
            if (mainStream)
                obj = new JArray(new JArray(array), null, null, true);
	*/
	ListValue *lv_26 = new ListValue;
	ListValue *lv_26_in = new ListValue;
	ListValueAppend(lv_26_in, 4,
		Value::CreateIntegerValue(5),
		Value::CreateIntegerValue(3),
		Value::CreateIntegerValue(4),
		Value::CreateIntegerValue(1)
	);
	lv_26->Append(lv_26_in);

	ListValue *sublv = new ListValue;
	ListValueAppend(sublv, 27,
		Value::CreateIntegerValue(1),
		Value::CreateIntegerValue(1),
		NULL, NULL, NULL, NULL, NULL,
		Value::CreateStringValue("social.google.com"),
		new ListValue,
		NULL, NULL, NULL, NULL, NULL, NULL,
		new ListValue,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		lv_26
	);

	ListValue lv;
	ListValueAppend(&lv, 4,
		sublv,
		NULL, NULL,
		Value::CreateBooleanValue(true)
	);

	std::string json;
	base::JSONWriter::Write(&lv, false, &json);

	CURLEncodeA urldata(json.c_str(), json.length());

	base::Properties pro;
	pro.Put("f.req", urldata.c_str());
	pro.Put("at", session_id.c_str());

	CBuffer htmlbuffer;
	CBuffer header;

	std::string url;
	FormatUrl(url, kGPGetActivities);

	if ((ret = RequestPost(url.c_str(), pro, htmlbuffer, &header)) != GP_OK) {
		return ret;
	}

	ret = -2;

	//cookie
	m_cookie_mgr.explain((char *)header.GetBuffer());

	if (!IsHttpOK((const char *)header.GetBuffer())) {
		return ret;
	}

	const char *res = (const char *)htmlbuffer.GetBuffer();
	if (strncmp(res, ")]}'\n\n", 6) == 0) res += 6;

	ListValue *list = (ListValue *)base::JSONReader::Read(res, true);
	
	if (list) {
		if (list->GetType() == Value::TYPE_LIST) {
			//list = (JArray)result.ResponseObject[0][1][1][0];
			ListValue *tl = NULL;
			bool bget = false;
			list->GetList(0, &tl);
			if (tl) {
				tl->GetList(1, &tl);
				if (tl) {
					tl->GetList(1, &tl);
					if (tl) {
						if (tl->GetList(0, NULL)) {
							tl->Remove(0, (Value **)lvActivities);
							bget = true;
						}
					}
				}
			}

			if (bget) {
				ret = GP_OK;
			}
		}
		delete list;
	}

	return ret;
}

int GooglePlus::UpdateLastReadTime(uint64 lasttime)
{
	int ret = -1;

	//f.req=%5B1356610887435402%5D&at=AObGSAgmcrpvLmbYT3dILZP036zGDz59Qw%3A1356610868763&
	//[1356610887435402]

	std::string postdata = "f.req=%5B";
	postdata += base::Int64ToString(lasttime);
	postdata += "%5D&at=";
	postdata += session_id;

	CBuffer htmlbuffer;
	CBuffer header;

	std::string url;
	FormatUrl(url, kGPUpdateLastReadTime);

	if ((ret = RequestPost(url.c_str(), postdata.c_str(), htmlbuffer, &header)) != GP_OK) {
		return ret;
	}

	ret = -2;

	//cookie
	m_cookie_mgr.explain((char *)header.GetBuffer());

	if (!IsHttpOK((const char *)header.GetBuffer())) {
		return ret;
	}

	ret = GP_OK;

	return ret;
}

int GooglePlus::GetNotificationsData(ListValue **lvNotifications, bool auto_update_last_read_time)
{
	int ret = -1;

	std::string postdata = "f.req=%5Bnull%2C%5B%5D%2C5%2Cnull%2C%5B%5D%2Cnull%2Ctrue%2C%5B%5D%2Cnull%2Cnull%2Cnull%2Cnull%2C2%5D&at=";
	postdata += session_id;

	CBuffer htmlbuffer;
	CBuffer header;

	std::string url;
	FormatUrl(url, kGPGetNotificationsData);

	if ((ret = RequestPost(url.c_str(), postdata.c_str(), htmlbuffer, &header)) != GP_OK) {
		return ret;
	}

	ret = -2;

	//cookie
	m_cookie_mgr.explain((char *)header.GetBuffer());

	if (!IsHttpOK((const char *)header.GetBuffer())) {
		return ret;
	}

	const char *res = (const char *)htmlbuffer.GetBuffer();
	if (strncmp(res, ")]}'\n\n", 6) == 0) res += 6;

	ListValue *list = (ListValue *)base::JSONReader::Read(res, true);
	
	if (list) {
		if (list->GetType() == Value::TYPE_LIST) {
			//JArray array = (JArray)JArray.Parse(result.RawData.Substring(6))[0][1][1];
			ListValue *tl = NULL;
			if (ListValueGet(list, (Value **)&tl, true, 3, 0, 1, 1)) {
				if (tl->GetType() == Value::TYPE_LIST) {
					*lvNotifications = (ListValue *)tl;
					//(long)(float)array[2]
					if (auto_update_last_read_time) {
						double t = 0;
						if (tl->GetReal(2, &t)) {
							UpdateLastReadTime((uint64)t);
						}
					}
					ret = GP_OK;
				} else {
					delete tl;
				}
			}
		}
		delete list;
	}

	return ret;
}

int GooglePlus::PlusOnePost(LPCSTR postId, bool set)
{
	int ret = -1;

	//itemId=buzz%3Az134hh3a1ojegbf3i23ld10rvl3dvfwqv04&set=true&cdcx=e&cdcy=2&cdsx=16&cdsy=e&at=AObGSAgmcrpvLmbYT3dILZP036zGDz59Qw%3A1356610868763&
	//array = "itemId=buzz%3A" + postId + "&set=" + set.ToString() + "&cdcx=b&cdcy=a&cdsx=16&cdsy=e&at=" + App.Session.SessionID;

	std::string postdata = "itemId=buzz%3A";
	postdata += postId;
	postdata += "&set=";
	postdata += (set ? "true" : "false");
	postdata += "&cdcx=b&cdcy=a&cdsx=16&cdsy=e&at=";
	postdata += session_id;

	CBuffer htmlbuffer;
	CBuffer header;

	std::string url;
	FormatUrl(url, kGPPlusOne);

	if ((ret = RequestPost(url.c_str(), postdata.c_str(), htmlbuffer, &header)) != GP_OK) {
		return ret;
	}

	ret = -2;

	//cookie
	m_cookie_mgr.explain((char *)header.GetBuffer());

	if (!IsHttpOK((const char *)header.GetBuffer())) {
		return ret;
	}

	ret = GP_OK;

	return ret;
}

int GooglePlus::PlusOneComment(LPCSTR postId, LPCSTR commentId, bool set)
{
	int ret = -1;

	//itemId=comment%3Az12cwfyy0lrmgfho004cfhuj0p3ux5ko2yc0k%231356611174238575&set=true&at=AObGSAgmcrpvLmbYT3dILZP036zGDz59Qw%3A1356610868763&
	//array = "itemId=comment%3A" + commentId + "&set=" + set.ToString() + "&cdcx=b&cdcy=3&cdsx=18&cdsy=f&at=" + App.Session.SessionID;

	std::string postdata = "itemId=comment%3A";
	if (postId) {
		postdata += postId;
		postdata += "%23";
		postdata += commentId;
	} else {
		CURLEncodeA encodeCommentId(commentId, lstrlenA(commentId));
		postdata += encodeCommentId.c_str();
	}
	postdata += "&set=";
	postdata += (set ? "true" : "false");
	postdata += "&cdcx=b&cdcy=3&cdsx=18&cdsy=f&at=";
	postdata += session_id;

	CBuffer htmlbuffer;
	CBuffer header;

	std::string url;
	FormatUrl(url, kGPPlusOne);

	if ((ret = RequestPost(url.c_str(), postdata.c_str(), htmlbuffer, &header)) != GP_OK) {
		return ret;
	}

	ret = -2;

	//cookie
	m_cookie_mgr.explain((char *)header.GetBuffer());

	if (!IsHttpOK((const char *)header.GetBuffer())) {
		return ret;
	}

	ret = GP_OK;

	return ret;
}

int GooglePlus::Comment(LPCSTR postId, LPCWSTR commentContent)
{
	int ret;
	char *utf8Comment = NULL;
	lo_W2Utf8(&utf8Comment, commentContent);

	ret = Comment(postId, utf8Comment);

	free(utf8Comment);

	return ret;
}

int GooglePlus::Comment(LPCSTR postId, LPCSTR utf8commentContent)
{
	int ret = -1;
	time_t t = time(NULL) * 1000;

	//obj = new JArray(postId, "os:" + postId + ":" + Utils.Time, commentContent, Utils.Time + 4000, null, null, 4);
	ListValue lv;

	ListValueAppend(&lv, 7,
		Value::CreateStringValue(postId),
		Value::CreateStringValue("os:" + std::string(postId) + ":" + base::Int64ToString(t)),
		Value::CreateStringValue(utf8commentContent),
		Value::CreateStringValue(base::Int64ToString(t + 4000)),
		NULL, NULL,
		Value::CreateIntegerValue(1)
	);

	std::string json;
	base::JSONWriter::Write(&lv, false, &json);

	CURLEncodeA urldata(json.c_str(), json.length());

	base::Properties pro;
	pro.Put("f.req", urldata.c_str());
	pro.Put("at", session_id.c_str());

	CBuffer htmlbuffer;
	CBuffer header;

	std::string url;
	FormatUrl(url, kGPComment);

	if ((ret = RequestPost(url.c_str(), pro, htmlbuffer, &header)) != GP_OK) {
		return ret;
	}

	ret = -2;

	//cookie
	m_cookie_mgr.explain((char *)header.GetBuffer());

	if (!IsHttpOK((const char *)header.GetBuffer())) {
		return ret;
	}

	ret = GP_OK;

	return ret;
}

int GooglePlus::RequestPost(LPCSTR url, base::Properties& pro, CBuffer& response, CBuffer *header)
{
	std::string postdata;
	pro.BuildProperties("&", postdata);

	return RequestPost(url, postdata.c_str(), response, header);
}

curl_slist* GooglePlus::post_addheader(curl_slist *list)
{
	list = CTANetBase::post_addheader(list);
	list = curl_slist_append(list, "Origin: https://plus.google.com");
	return list;
}

void GooglePlus::post_setopt(CURL *curl)
{
	CTANetBase::post_setopt(curl);
	curl_easy_setopt(curl, CURLOPT_REFERER, "https://plus.google.com/");
}

int GooglePlus::RequestPost(LPCSTR url, LPCSTR postdata, CBuffer& response, CBuffer *header)
{
	int ret = CurlRequestPost(url, postdata, response, header);

	if (ret == TA_NET_OK) {
		response.WriteZeroByte();
		return GP_OK;
	}

	return ret;
}

int GooglePlus::RequestGet(LPCSTR url, CBuffer& response, CBuffer *header)
{
	// GP_OK == TA_NET_OK

	int ret = CurlRequestGet(url, response, header);

	if (ret == TA_NET_OK) {
		response.WriteZeroByte();
		return GP_OK;
	}

	return ret;
}