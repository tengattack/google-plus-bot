
#ifndef _GOOGLE_PLUS_TYPES_MEDIA_H_
#define _GOOGLE_PLUS_TYPES_MEDIA_H_ 1
#pragma once

#include <string>
#include <vector>
#include <base/string/values.h>

class GooglePlus;

namespace gplus {

enum MediaType {
	kMTUnknow = 0,
	kMTLink,
	kMTImage,
};

class Media {
public:
	Media(ListValue *lv = NULL);
	~Media();

	bool parse(ListValue *lv);

	void clear();

	std::vector<std::string> m_images_url;
	std::string m_link_url;

	inline MediaType GetType() {
		return m_type;
	}

protected:
	MediaType m_type;
};

}

#endif