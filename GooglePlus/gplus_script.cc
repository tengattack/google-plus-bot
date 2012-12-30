
#include "gplus_script.h"

#include <v8.h>

#include <common/strconv.h>
#include <common/Buffer.h>

#include <engine/script_engine.h>

#include "gplustypes/post.h"
#include "gplustypes/comment.h"

using namespace v8;

CTAScriptEngine tae;

CScriptGooglePlus *g_gplus = NULL;

#include "gplus_script_class.h"

void SetGlobalGooglePlus(CScriptGooglePlus *gp)
{
	g_gplus = gp;
}

/*
v8::Handle<v8::Object> V8Value(::Value *val)
{
	HandleScope handle_scope;
	if (!val) {
		return handle_scope.Close(v8::Undefined());
	}
	switch (val->GetType()) {
	case ::Value::TYPE_NULL:
		return handle_scope.Close(v8::Null());
		break;
	case ::Value::TYPE_BOOLEAN:
		{
			bool bret = false;
			val->GetAsBoolean(&bret);
			return handle_scope.Close(bret ? v8::True() : v8::False());
		}
		break;
	case ::Value::TYPE_INTEGER:
		{
			int iret = 0;
			val->GetAsInteger(&iret);
			return handle_scope.Close(v8::Int32::New(iret));
		}
		break;
	case ::Value::TYPE_REAL:
		{
			double dret = 0;
			val->GetAsReal(&dret);
			return handle_scope.Close(v8::Number::New(dret));
		}
		break;
	case ::Value::TYPE_STRING:
		{
			std::string strret = 0;
			val->GetAsString(&strret);
			return handle_scope.Close(v8::String::New(strret.c_str(), strret.length()));
		}
		break;
	case ::Value::TYPE_BINARY:
		{
			char *buffer = ((BinaryValue *)val)->GetBuffer();
			size_t length = ((BinaryValue *)val)->GetSize();
			return handle_scope.Close(v8::String::New(buffer, length));
		}
		break;
	case ::Value::TYPE_DICTIONARY:
		{
			v8::Handle<v8::Object> obj = v8::Object::New();
			DictionaryValue::key_iterator iter = ((DictionaryValue *)val)->begin_keys();
			
			while (iter != ((DictionaryValue *)val)->end_keys()) {
				//(*iter).
				::Value *v = NULL;
				if (((DictionaryValue *)val)->Get(*iter, &v)) {
					obj->Set(String::New((*iter).c_str()), V8Value(v));
				} else {
					obj->Set(String::New((*iter).c_str()), v8::Undefined());
				}
			}

			return obj;
		}
		break;
	case ::Value::TYPE_LIST:
		{
			v8::Handle<v8::Array> arr = v8::Array::New();
			ListValue::iterator iter = ((ListValue *)val)->begin();
			
			uint32 i = 0;
			while (iter != ((ListValue *)val)->end()) {
				arr->Set(i, V8Value(*iter));
				i++;
			}
		}
		break;
	}

	return handle_scope.Close(v8::Undefined());
}
*/

void SetPostBaseInfoToObject(gplus::Post *post, v8::Handle<v8::Object> obj_post)
{
	HandleScope handle_scope;

	obj_post->Set(String::New("id"), String::New(post->id.c_str()));
	obj_post->Set(String::New("author"), String::New(post->author.c_str()));
	obj_post->Set(String::New("author_id"), String::New(post->author_id.c_str()));
	obj_post->Set(String::New("content"), String::New(post->content.c_str()));

	obj_post->Set(String::New("in_community"), Boolean::New(post->m_in_community));

	if (post->m_in_community) {
		v8::Handle<v8::Object> obj_community = Object::New();

		obj_community->Set(String::New("name"), String::New(post->m_community_name.c_str()));
		obj_community->Set(String::New("category"), String::New(post->m_community_category.c_str()));

		obj_post->Set(String::New("community"), obj_community);
	}

	v8::Handle<v8::Object> obj_media = Object::New();
	obj_media->Set(String::New("type"), Int32::New(post->m_media.GetType()));
	switch (post->m_media.GetType()) {
	case gplus::kMTLink:
		obj_media->Set(String::New("link_url"), String::New(post->m_media.m_link_url.c_str()));
		break;
	case gplus::kMTImage:
		{
			v8::Handle<v8::Array> obj_images = Array::New();
			for (int i = 0; i < post->m_media.m_images_url.size(); i++) {
				obj_images->Set(i, String::New(post->m_media.m_images_url[i].c_str()));
			}
			obj_media->Set(String::New("images"), obj_images);
		}
		break;
	}

	obj_post->Set(String::New("media"), obj_media);

	obj_post->Set(String::New("is_reshare"), Boolean::New(post->is_reshare));
}

void AddGlobalObject(UserParam *up, v8::Handle<v8::Object> obj)
{
	HandleScope handle_scope;

	v8::Handle<v8::Object> obj_baseinfo = Object::New();
	obj_baseinfo->Set(String::New("user"), String::New(up->user.c_str()));
	obj_baseinfo->Set(String::New("user_id"), String::New(up->user_id.c_str()));
	obj_baseinfo->Set(String::New("post_id"), String::New(up->post_id.c_str()));
	obj_baseinfo->Set(String::New("author_id"), String::New(up->author_id.c_str()));
	obj_baseinfo->Set(String::New("content"), String::New(up->content.c_str()));
	obj_baseinfo->Set(String::New("author"), String::New(up->author.c_str()));

	obj->Set(String::New("baseinfo"), obj_baseinfo);

	v8::Handle<v8::Object> obj_post = Object::New();
	if (up->post) {

		SetPostBaseInfoToObject(up->post, obj_post);

		v8::Handle<v8::Array> obj_comments = Array::New();
		for (int i = 0; i < up->post->m_comment_count; i++) {
			gplus::Comment *comment = up->post->GetComment(i);

			v8::Handle<v8::Object> obj_comment = Object::New();
			if (comment) {
				obj_comment->Set(String::New("id"), String::New(comment->id.c_str()));
				obj_comment->Set(String::New("author"), String::New(comment->author.c_str()));
				obj_comment->Set(String::New("author_id"), String::New(comment->author_id.c_str()));
				obj_comment->Set(String::New("content"), String::New(comment->content.c_str()));
			}
			obj_comments->Set(i, obj_comment);
		}
		obj_post->Set(String::New("comments"), obj_comments);

		if (up->post->is_reshare && up->post->original) {
			v8::Handle<v8::Object> obj_original = Object::New();

			SetPostBaseInfoToObject(up->post->original, obj_original);
			obj_post->Set(String::New("original"), obj_original);
		}
	}

	obj->Set(String::New("post"), obj_post);
}

void ScriptCallback(void *user, v8::Handle<v8::ObjectTemplate> templ, v8::Handle<v8::Object> *obj, v8::Handle<v8::Script> *script, ScriptCallbackStatus status)
{
	UserParam *up = (UserParam *)user;
	if (!up) {
		return;
	}

	switch (status) {
	case kSCSInit:
		AddClsGooglePlus(templ);
		break;
	case kSCSContext:
		AddGlobalObject(up, *obj);
		SetEvent(up->e);
		break;
	case kSCSError:
		{
			
		}
		break;
	case kSCSFinish:
		delete up;
		break;
	}
}
