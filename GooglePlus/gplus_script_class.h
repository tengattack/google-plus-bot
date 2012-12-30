
#ifndef _TA_GOOGLE_PLUS_SCRIPT_H_
#define _TA_GOOGLE_PLUS_SCRIPT_H_
//-----------GooglePlus for V8Engine--------------

#include "GooglePlus.h"

#include <engine/script_engine.h>

using namespace v8;

CScriptGooglePlus* GetGooglePlus(const Arguments& args)
{
	HandleScope handle_scope;

	Handle<Object> obj = args.Holder();
	CScriptGooglePlus* pgp = (CScriptGooglePlus *)obj->GetPointerFromInternalField(0);

	if (!pgp) {
		pgp = g_gplus;
	}

	return pgp;
}

Handle<v8::Value> ClsGooglePlusConstructor(const Arguments& args)
{
	HandleScope handle_scope;
	Handle<Object> obj = args.This();

	CScriptGooglePlus *pgp = new CScriptGooglePlus;
	obj->SetInternalField(0, External::New(pgp));
	return obj;
}

Handle<v8::Value> ClsGooglePlus_GetBaseInfo(const Arguments& args)
{
	HandleScope handle_scope;

	CScriptGooglePlus* pgp = GetGooglePlus(args);

	if (!pgp) return v8::Undefined();

	if (pgp->GetBaseInfo() == GP_OK) {
		return v8::True();
	} else {
		return v8::False();
	}
}

Handle<v8::Value> ClsGooglePlus_SetPage(const Arguments& args)
{
	HandleScope handle_scope;

	CScriptGooglePlus* pgp = GetGooglePlus(args);

	if (!pgp) return v8::Undefined();

	if (args.Length() > 0)
	{
		char *pageId = NULL, *pageName = NULL;
		
		Handle<String> v8s_pageName = args[0]->ToString();
		lo_V8S2Utf8(&pageName, v8s_pageName);

		if (args.Length() > 1) {
			Handle<String> v8s_pageId = args[1]->ToString();
			lo_V8S2C(&pageId, v8s_pageId);
		}

		if (pageId && lstrlenA(pageId) <= 0) {
			//clear page

			free(pageId);
			pageId = NULL;
		}

		pgp->SetPage(pageName, pageId);
		free(pageId);
		free(pageName);
	}
	return v8::Undefined();
}

Handle<v8::Value> ClsGooglePlus_PlusOnePost(const Arguments& args)
{
	HandleScope handle_scope;

	CScriptGooglePlus* pgp = GetGooglePlus(args);

	if (!pgp) return v8::Undefined();

	if (args.Length() > 0)
	{
		bool bret;
		bool bset = true;
		char *postId = NULL;
		Handle<String> v8s_postId = args[0]->ToString();

		lo_V8S2C(&postId, v8s_postId);

		if (args.Length() > 1) {
			bset = args[1]->BooleanValue();
		}

		bret = (pgp->PlusOnePost(postId, bset) == GP_OK);
		free(postId);

		if (bret) {
			return v8::True();
		} else {
			return v8::False();
		}
	}
	return v8::Undefined();
}

Handle<v8::Value> ClsGooglePlus_PlusOneComment(const Arguments& args)
{
	HandleScope handle_scope;

	CScriptGooglePlus* pgp = GetGooglePlus(args);

	if (!pgp) return v8::Undefined();

	if (args.Length() > 1)
	{
		bool bret;
		bool bset = true;
		char *postId = NULL;
		char *commentId = NULL;

		Handle<String> v8s_postId = args[0]->ToString();
		Handle<String> v8s_commentId = args[1]->ToString();

		lo_V8S2C(&postId, v8s_postId);
		lo_V8S2C(&commentId, v8s_commentId);

		if (postId && lstrlenA(postId) <= 0) {
			free(postId);
			postId = NULL;
		}

		if (args.Length() > 2) {
			bset = args[2]->BooleanValue();
		}

		bret = (pgp->PlusOneComment(postId, commentId, bset) == GP_OK);
		free(postId);
		free(commentId);

		if (bret) {
			return v8::True();
		} else {
			return v8::False();
		}
	}
	return v8::Undefined();
}

Handle<v8::Value> ClsGooglePlus_Comment(const Arguments& args)
{
	HandleScope handle_scope;

	CScriptGooglePlus* pgp = GetGooglePlus(args);

	if (!pgp) return v8::Undefined();

	if (args.Length() > 1)
	{
		bool bret;
		char *postId = NULL, *utf8comment = NULL;

		Handle<String> v8s_postId = args[0]->ToString();
		Handle<String> v8s_comment = args[1]->ToString();

		lo_V8S2C(&postId, v8s_postId);
		lo_V8S2Utf8(&utf8comment, v8s_comment);

		if (postId && utf8comment) {
			bret = (pgp->Comment(postId, utf8comment) == GP_OK);
		}
		free(postId);
		free(utf8comment);

		if (bret) {
			return v8::True();
		} else {
			return v8::False();
		}
	}
	return v8::Undefined();
}

Handle<v8::Value> ClsGooglePlus_Login(const Arguments& args)
{
	HandleScope handle_scope;

	CScriptGooglePlus* pgp = GetGooglePlus(args);

	if (!pgp) return v8::Undefined();

	if (args.Length() > 1)
	{
		bool bret;
		char *email = NULL;
		char *password = NULL;

		Handle<String> v8s_email = args[0]->ToString();
		Handle<String> v8s_password = args[1]->ToString();

		lo_V8S2C(&email, v8s_email);
		lo_V8S2C(&password, v8s_password);

		bret = (pgp->Login(email, password) == GP_OK);
		free(email);
		free(password);

		if (bret) {
			return v8::True();
		} else {
			return v8::False();
		}
	}
	return v8::Undefined();
}

Handle<v8::Value> ClsGooglePlus_Dispose(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	CScriptGooglePlus* pgp = (CScriptGooglePlus *)self->GetPointerFromInternalField(0);
	
	if (pgp && pgp != g_gplus) {
		delete pgp;
		self->SetInternalField(0, External::New(NULL));
	}

	return v8::Undefined();
}

void SetClsGooglePlusPrototype(Handle<ObjectTemplate> obj)
{
	obj->Set("PlusOnePost", FunctionTemplate::New(ClsGooglePlus_PlusOnePost));
	obj->Set("PlusOneComment", FunctionTemplate::New(ClsGooglePlus_PlusOneComment));

	obj->Set("Comment", FunctionTemplate::New(ClsGooglePlus_Comment));
	obj->Set("GetBaseInfo", FunctionTemplate::New(ClsGooglePlus_GetBaseInfo));
	obj->Set("Login", FunctionTemplate::New(ClsGooglePlus_Login));

	obj->Set("SetPage", FunctionTemplate::New(ClsGooglePlus_SetPage));
	
	//obj->Set("Free", FunctionTemplate::New(ClsGooglePlus_Free));
	obj->Set("dispose", FunctionTemplate::New(ClsGooglePlus_Dispose));
}

void AddClsGooglePlus(Handle<ObjectTemplate> global)
{
	HandleScope handle_scope;

	Handle<FunctionTemplate> cls = FunctionTemplate::New(ClsGooglePlusConstructor);

	cls->SetClassName(String::New("CGooglePlus"));
	global->Set(String::New("CGooglePlus"), cls);

	Handle<ObjectTemplate> proto = cls->PrototypeTemplate();
	SetClsGooglePlusPrototype(proto);

	Handle<ObjectTemplate> inst = cls->InstanceTemplate();
	inst->SetInternalFieldCount(1);

	CScriptBase::AddBaseFunction(inst);


	Handle<ObjectTemplate> gpobj = ObjectTemplate::New();

	gpobj->SetInternalFieldCount(1);

	SetClsGooglePlusPrototype(gpobj);
	CScriptBase::AddBaseFunction(gpobj);

	global->Set("gplus", gpobj);
}

#endif //_TA_GOOGLE_PLUS_SCRIPT_H_