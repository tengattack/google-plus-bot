
#include <time.h>
#include "cache.h"

void ClearOldCache(CACHE_TABLE *cache, uint32 max_time)
{
	time_t current_time = time(NULL);
	CACHE_TABLE::iterator iter = cache->begin();
	while (iter != cache->end()) {
		if (iter->second.t < current_time - max_time) {
			iter = cache->erase(iter);
		} else {
			iter++;
		}
	}
}
