
#include "config.h"

#include "rule.h"

#include <filecommon/file_path.h>

#include <base/file/file.h>
#include <base/file/filedata.h>

#include <base/string/string_split.h>
#include <base/string/string_tokenizer.h>
#include <base/string/values.h>

#include <base/json/json_writer.h>
#include <base/json/json_reader.h>

std::string& replace_all(std::string& str, const std::string& old_value, const std::string& new_value)
{   
	while (true)
	{
		std::string::size_type pos(0);
		if ((pos=str.find(old_value)) != std::string::npos)
			str.replace(pos, old_value.length(), new_value);
		else
			break;
	}
	return str;
}

namespace config {

std::string email;
std::string password;

int imode = 0;
int refresh_time = 1000;
int cache_time = (60 * 30);

std::string page_id;
std::string page_name;

std::string community_id;

std::string username;

std::wstring base_path;
std::wstring config_path;
std::wstring rulepath;
std::wstring cookiepath;
std::wstring lasttimepath;
std::wstring jsfolderpath;
std::wstring script_base_path;

std::wstring initscript;

bool JsonToValue(std::string& json, Value **val)
{
	int err_code = 0;
	std::string err_msg;
	Value* v = base::JSONReader::ReadAndReturnError(json, true, &err_code, &err_msg);
	if (v) {
		*val = v;
		return true;
	} else {
		printf("Read failed, error code: %d\n%s\n", err_code, err_msg.c_str());
		return false;
	}
}

bool ReadJsonFile(LPCTSTR filepath, Value **val)
{
	bool bret = false;
	base::CFile file;
	if (file.Open(base::kFileRead, filepath)) {
		base::CFileData fd;
		if (fd.Read(file)) {
			std::string strraw;
			fd.ToUtf8Text(strraw);

			if (strraw.length()) {

				std::string strsjon;
				//处理注释
				StringTokenizer t(strraw, "\n");
				bool in_note = false;
				//t.set_quote_chars("\"");

				//这里没有检查注释在引号中的情况
				while (t.GetNext()) {

					std::string line = t.token();

					while (true) {
						const char *base = line.c_str();
						const char *note1 = NULL;
						const char *note2 = in_note ? strstr(base, "*/") : strstr(base, "/*");
					
						if (!in_note) note1 = strstr(base, "//");

						if (!in_note) {
							if (note1 && (note1 < note2 || !note2)) {
								// //注释在前面
								strsjon += line.substr(0, note1 - base);
							} else if (note2) {
								strsjon += line.substr(0, note2 - base);

								//同行
								const char *note3 = strstr(note2 + 2, "*/");
								if (note3) {
									line = note3 + 2;
									continue;
								} else {
									in_note = true;
								}
							} else {
								strsjon += line;
							}
						} else if (note2) {
							strsjon += line.substr(note2 + 2 - base);
							in_note = false;
						}

						break;
					}

					strsjon += "\n";
				}

				bret = JsonToValue(strsjon, val);
			}
		}
		file.Close();
	}
	return bret;
}

void LoadPath(std::wstring& path, LPCTSTR file, LPCTSTR defaultfile, bool isfolder = false)
{
	if (file && lstrlen(file) > 0) {
		//绝对路径
		if (wcsstr(file, L":\\")) {
			path = file;
		} else {
			path = base_path;
			//去掉最前面的路径分隔
			if (file[0] == '/' || file[0] == '\\') {
				path += file + 1;
			} else {
				path += file;
			}
		}
	} else {
		path = base_path;
		path += defaultfile;
	}
	if (isfolder) {
		int len = path.length();
		if (path[len - 1] != '/' && path[len - 1] != '\\') {
			path += L"\\";
		}
	}
}

bool LoadConfig(LPCTSTR confpath)
{
	TCHAR path[MAX_PATH] = {0};
	GetModuleFileName(NULL, path, MAX_PATH);

	base_path = GetFilePath(path);

	LoadPath(config_path, confpath, L"base.conf");
	
	bool bread = false;
	DictionaryValue *dv = NULL;

	wprintf(L"Loading %s...\n", config_path.c_str());
	if (ReadJsonFile(config_path.c_str(), (Value **)&dv)) {
		if (dv->GetType() == Value::TYPE_DICTIONARY) {
			ListValue *lv = NULL;

			dv->GetString("email", &email);
			dv->GetString("password", &password);

			dv->GetInteger("refresh_time", &refresh_time);
			dv->GetInteger("cache_time", &cache_time);

			dv->GetInteger("mode", &imode);

			dv->GetString("page.id", &page_id);
			dv->GetString("page.name", &page_name);

			dv->GetString("community.id", &community_id);

			dv->GetString("username", &username);

			std::wstring filepath;
			dv->GetString("script.folder", &filepath);
			LoadPath(jsfolderpath, filepath.c_str(), L"js", true);

			filepath.clear();
			dv->GetString("script.basepath", &filepath);
			LoadPath(script_base_path, filepath.c_str(), L"", true);

			dv->GetString("script.init", &initscript);

			filepath.clear();
			dv->GetString("path.rule", &filepath);
			LoadPath(rulepath, filepath.c_str(), L"rule.conf");

			filepath.clear();
			dv->GetString("path.cookie", &filepath);
			LoadPath(cookiepath, filepath.c_str(), L"cookie.txt");

			filepath.clear();
			dv->GetString("path.lasttime", &filepath);
			LoadPath(lasttimepath, filepath.c_str(), L"lasttime.txt");

			bread = true;
		}
		delete dv;
		dv = NULL;
	}

	if (!bread) {
		printf("LoadBaseConfig failed!\n");
		return false;
	}

	bread = false;

	wprintf(L"Loading %s...\n", rulepath.c_str());
	if (ReadJsonFile(rulepath.c_str(), (Value **)&dv)) {

		int rcount = LoadRuleTable(dv);
		if (rcount > 0) {
			printf("RuleCount: %d\n", rcount);
			bread = true;
		}

		delete dv;
		dv = NULL;
	}

	if (!bread) {
		printf("LoadRuleTable failed!\n");
		return false;
	}

	return true;
}

void CheckUpdate()
{
	
}

}