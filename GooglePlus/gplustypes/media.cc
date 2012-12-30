
#include "media.h"

#include "../GooglePlus.h"

#include <base/string/values_op.h>

namespace gplus {

Media::Media(ListValue *lv)
	: m_type(kMTUnknow)
{
	if (lv) parse(lv);
}

Media::~Media() {
}

void Media::clear()
{
	m_type = kMTUnknow;

	m_link_url.clear();

	m_images_url.clear();
}

bool Media::parse(ListValue *lv) {

	clear();

	if (lv->GetType() == Value::TYPE_LIST) {
		ListValue *sublv = NULL;
		ListValue *mediaRoot = NULL;
		DictionaryValue *subdv = NULL;

		int type = 0;

		if (lv->GetList(0, &sublv)) {
			sublv->GetInteger(0, &type);
		}

		switch (type) {
		case 35:
			m_type = kMTLink;
			if (lv->GetDictionary(2, &subdv)) {
				if (subdv->GetList("29646191", &mediaRoot)) {
					mediaRoot->GetString(0, &m_link_url);
				}
			}
			break;
		case 249:
			m_type = kMTImage;
			if (lv->GetDictionary(1, &subdv)) {
				if (subdv->GetList("27639957", &mediaRoot)) {
					std::string url;
					if (mediaRoot->GetString(6, &url)) {
						m_images_url.push_back(url);
					}
					/*if (ListValueGet(mediaRoot, (Value **)&sublv, false, 2, 0, 5)) {
						std::string url;
						if (sublv->GetString(0, &url)) {
							m_images_url.push_back(url);
						}
					}*/
				}
			}
			break;
		case 250:
			m_type = kMTImage;
			if (lv->GetDictionary(1, &subdv)) {
				subdv->GetList("27847199", &mediaRoot);
			} else {
				if (lv->GetDictionary(2, &subdv)) {
					subdv->GetList("27847199", &mediaRoot);
					//相册的样子
				}
			}
			if (mediaRoot && mediaRoot->GetList(3, &sublv)) {
				ListValue::iterator iter = sublv->begin();
				while (iter != sublv->end()) {
					std::string url;
					if ((*iter)->GetType() == Value::TYPE_LIST &&
						((ListValue *)*iter)->GetString(6, &url)) {
						m_images_url.push_back(url);
					}
					iter++;
				}
			}
			break;
		}
		/*
		JToken mediaRoot;
        switch ((int)media[0][0])
        {
            case 35:
                mediaRoot = (JArray)media[2]["29646191"];
                Icon = new Uri(prefix.FormatWith(mediaRoot[6]), UriKind.Absolute);
                if (mediaRoot[5].Type == JTokenType.Array)
                    Images.Add(new Uri(prefix.FormatWith(((string)mediaRoot[5][0]).Replace("resize_h=150&resize_w=150", "resize_h=400&resize_w=400")), UriKind.Absolute));
                LineOne = (string)mediaRoot[2];
                LineTwo = (string)mediaRoot[3];
                LinkLocation = new Uri((string)mediaRoot[0], UriKind.Absolute);
                break;
            case 249:
                mediaRoot = (JArray)media[1]["27639957"];
                Images.Add(new Uri(prefix.FormatWith(mediaRoot[0][5][0]), UriKind.Absolute));
                break;
            case 250:
                if (media[1].Type == JTokenType.Object)
                    mediaRoot = (JArray)media[1]["27847199"];
                else
                {
                    mediaRoot = (JArray)media[2]["27847199"];
                    LineOne = (string)mediaRoot[1];
                    LinkLocation = new Uri("//Pages/Album.xaml?uid={0}&aid={1}".FormatWith(mediaRoot[4], mediaRoot[5]), UriKind.Relative);
                }
                foreach (JToken item in mediaRoot[3])
                    Images.Add(new Uri(prefix.FormatWith(item[0][5][0]), UriKind.Absolute));
                break;
        }
		*/


	}
	return false;
}

}