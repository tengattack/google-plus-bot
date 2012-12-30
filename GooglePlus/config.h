
#ifndef _TA_CONFIG_
#define _TA_CONFIG_ 1
#pragma once

#include <base/basictypes.h>
#include <windows.h>
#include <string>

class Value;

namespace config {

extern std::string email;
extern std::string password;

extern int imode;
extern int refresh_time;
extern int cache_time;

extern std::string page_id;
extern std::string page_name;

extern std::string community_id;

extern std::string username;

extern std::wstring base_path;
extern std::wstring config_path;
extern std::wstring rulepath;
extern std::wstring cookiepath;
extern std::wstring lasttimepath;
extern std::wstring jsfolderpath;
extern std::wstring script_base_path;

extern std::wstring initscript;

bool ReadJsonFile(LPCTSTR filepath, Value **json);
bool LoadConfig(LPCTSTR path);

void CheckUpdate();

}

#define CACHE_MAX_TIME config::cache_time
#define REFRESH_TIME config::refresh_time

#endif