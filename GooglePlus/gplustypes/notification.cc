
#include "notifications.h"

#include <base/string/values.h>
#include <base/string/values_op.h>

#include "post.h"

namespace gplus {

Notification::Notification(ListValue *lv, bool unread)
	: m_unread(unread)
	, m_type(kNTUnknow)
	, m_type_do(kNTDUnknow)
	, m_post(NULL)
{
	if (lv) parse(lv);
}

Notification::~Notification()
{
	
}

void Notification::clear()
{
	if (m_post) {
		delete m_post;
		m_post = NULL;
	}
}

bool Notification::parse(ListValue *lv)
{
	/*Type = (NotificationType)(int)notification[1];
    ID = (string)notification[10];*/
	clear();

	int type = 0;
	lv->GetInteger(1, &type);
	lv->GetString(10, &m_id);

	if (type < kNTCount) {
		m_type = (NotificationType)type;

		switch (m_type) {
		case kNTPost:
			//if (notification[18][0][0].Type != JTokenType.Undefined)
			{
				std::string author_id;
				ListValue *sublv = NULL;
				if (ListValueGet(lv, (Value **)&sublv, false, 3, 18, 0, 0)) {
					//if (notification[18][0][7][1].Type != JTokenType.Undefined)
					if (ListValueGet(lv, NULL, false, 4, 18, 0, 7, 1)) {
						m_post = new Post(sublv);
					}

					//if (notification[18][0][7][2].Type != JTokenType.Undefined)
					StringValue *sv = NULL;
					if (ListValueGet(lv, (Value **)&sv, false, 4, 18, 0, 7, 2)) {
						//image
						/*string lineTwoId = (string)notification[18][0][7][3];
                        foreach (JToken block in notification[2])
							foreach (JToken item in block[1])*/
						if (sv->GetType() == Value::TYPE_STRING) {
							sv->GetAsString(&m_content);
						}

						if (ListValueGet(lv, (Value **)&sv, false, 4, 18, 0, 7, 3)) {
							if (sv->GetType() == Value::TYPE_STRING) {
								sv->GetAsString(&m_author_id);
							}
						}
					}

					ListValue *block = NULL, *subblock = NULL;
					if (lv->GetList(2, &block)) {
						ListValue::iterator iter = block->begin();

						while (iter != block->end()) {
		
							int blocktype = 0;
							if (((ListValue *)*iter)->GetInteger(0, &blocktype)) {
								switch (blocktype) {
								case 4:
									break;
								case 5:
									m_type_do = kNTDMention;
									break;
								case 6:
									if (ListValueGet(((ListValue *)*iter), (Value **)&subblock, false, 4, 1, 0, 2, 0)) {
										if (subblock->GetType() == Value::TYPE_STRING) {
											subblock->GetAsString(&m_author);
										}
									}
									m_type_do = kNTDReshare;
									break;
								case 11:
									m_type_do = kNTDComment;
									break;
								}
							}

							if (((ListValue *)*iter)->GetList(1, &subblock)) {
								ListValue::iterator subiter = subblock->begin();

								while (subiter != subblock->end()) {
									/*if ((string)item[2][3] == lineTwoId)
									{
										Image = new Uri("https:" + ((string)item[2][2]).Replace("photo.", "s64-c-k/photo."), UriKind.Absolute);
										LineTwo.Inlines.Add(new Run() { Text = (string)item[2][0], FontWeight = FontWeights.Bold });
										LineTwo.Inlines.Add(new Run() { Text = " - " });
										foreach (Inline inline in Utils.ProcessRawContent((string)notification[18][0][7][2]).Inlines)
											if (inline is Run)
												LineTwo.Inlines.Add(RichTextHelper.Clone((Run)inline));
										return;
									}*/

									StringValue *subsv = NULL;
									int itemtype = 0;

									if (blocktype == 4 && ((ListValue *)*subiter)->GetInteger(1, &itemtype)) {
										switch (itemtype) {
										case 20:
											m_type_do = kNTDPlusOnePost;
											break;
										case 21:
											m_type_do = kNTDPlusOneComment;
											break;
										}
									}

									if (itemtype == 20 || itemtype == 21) {
											if (ListValueGet(((ListValue *)*subiter), (Value **)&subsv, false, 2, 2, 0)) {
											if (subsv->GetType() == Value::TYPE_STRING) {
												subsv->GetAsString(&m_author);
											}
										}
										if (ListValueGet(((ListValue *)*subiter), (Value **)&subsv, false, 2, 2, 3)) {
											if (subsv->GetType() == Value::TYPE_STRING) {
												subsv->GetAsString(&author_id);
											}
										}
									} else if (ListValueGet(((ListValue *)*subiter), (Value **)&subsv, false, 2, 2, 3)) {
										if (subsv->GetType() == Value::TYPE_STRING) {
											std::string id;
											if (subsv->GetAsString(&id)) {
												if (id == m_author_id) {
													//comment id
													//m_author
													if (ListValueGet(((ListValue *)*subiter), (Value **)&subsv, false, 2, 2, 0)) {
														if (subsv->GetType() == Value::TYPE_STRING) {
															subsv->GetAsString(&m_author);
														}
													}

													//break here
													return true;
												}
											}
										}
									}
									subiter++;
								}
							}
							iter++;
						}
					}
                        /*switch ((int)block[0])
                        {
                            case 4:
                                foreach (JToken item in block[1])
                                {
                                    switch ((int)item[1])
                                    {
                                        case 20:
                                            LineTwo.Inlines.Add(new Run() { Text = "+1  ", FontStyle = FontStyles.Italic, FontWeight = FontWeights.Bold });
                                            LineTwo.Inlines.Add(new Run() { Text = "your post by " + (string)item[2][0] + ". " });
                                            Image = "https:" + ((string)item[2][2]).Replace("photo.", "s64-c-k/photo.");
                                            break;
                                        case 21:
                                            LineTwo.Inlines.Add(new Run() { Text = "+1  ", FontStyle = FontStyles.Italic, FontWeight = FontWeights.Bold });
                                            LineTwo.Inlines.Add(new Run() { Text = "your comment by " + (string)item[2][0] + ". " });
                                            Image = "https:" + ((string)item[2][2]).Replace("photo.", "s64-c-k/photo.");
                                            break;
                                    }
                                }
                                break;
                            case 6:
                                LineTwo.Inlines.Add(new Run() { Text = "1 new reshare ", FontStyle = FontStyles.Italic, FontWeight = FontWeights.Bold });
                                LineTwo.Inlines.Add(new Run() { Text = "by " + (string)block[1][0][2][0] + "." });
                                break;
                            case 11:
                                foreach (JToken item in block[1])
                                {
                                    switch ((int)item[1])
                                    {
                                        case 3:
                                            break;
                                    }
                                }
                                break;
                        }*/
				}	//[18][0][0]
				if (m_author_id.length() <= 0 && author_id.length() > 0) {
					m_author_id = author_id;
				}
			}
			break;
		case kNTCircle:
			break;
		}
	}

	return true;
}



}