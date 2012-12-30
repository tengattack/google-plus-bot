
#ifndef _TA_CACHE_
#define _TA_CACHE_ 1
#pragma once

#include <base/basictypes.h>

#include <string>
#include <map>

#include <common/wiseint.h>

typedef struct _CACHE {
	WiseInt wi;
	time_t t;
} CACHE;

typedef std::map<std::string, CACHE> CACHE_TABLE;

void ClearOldCache(CACHE_TABLE *cache, uint32 max_time);

#endif