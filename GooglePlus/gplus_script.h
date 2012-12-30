
#include "GooglePlus.h"

namespace gplus {
	class Post;
}

#include <engine/script_engine.h>
#include <engine/script/ScriptBase.h>
#include <v8.h>

#include <windows.h>

class CScriptGooglePlus : public GooglePlus, public CScriptBase {
public:
	CScriptGooglePlus() {}
	~CScriptGooglePlus() {}
};

typedef struct _UserParam {
	HANDLE e;
	GooglePlus *gp;
	gplus::Post *post;
	std::string user;
	std::string user_id;
	std::string author;
	std::string author_id;
	std::string content;
	std::string post_id;
} UserParam;

void SetGlobalGooglePlus(CScriptGooglePlus *gp);
void ScriptCallback(void *user, v8::Handle<v8::ObjectTemplate> templ, v8::Handle<v8::Object> *obj, v8::Handle<v8::Script> *script, ScriptCallbackStatus status);

extern CTAScriptEngine tae;

extern CScriptGooglePlus *g_gplus;
