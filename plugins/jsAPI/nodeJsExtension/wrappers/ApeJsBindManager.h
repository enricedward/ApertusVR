/*MIT License

Copyright (c) 2018 MTA SZTAKI

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#ifndef APE_JSBIND_MANAGER_H
#define APE_JSBIND_MANAGER_H

#include <array>
#include <string>
#include <map>
#include <iostream>
#include "nbind/nbind.h"
#include "nbind/api.h"
#include "ApeJsBindColor.h"
#include "ApeJsBindDegree.h"
#include "ApeJsBindEuler.h"
#include "ApeJsBindQuaternion.h"
#include "ApeJsBindRadian.h"
#include "ApeJsBindVector3.h"
#include "ApeJsBindMatrix4.h"
#include "ApeIScene.h"
#include "ApeIEventManager.h"
#include "ApeISystemConfig.h"
#include "ApeJsBindIndexedFaceSetGeometryImpl.h"
#include "ApeIndexedLineSetGeometryJsBind.h"
#include "ApeBoxGeometryJsBind.h"
#include "ApeFileGeometryJsBind.h"
#include "ApeJsBindLightImpl.h"
#include "ApeJsBindNodeImpl.h"
#include "ApeJsBindTextGeometryImpl.h"
#include "ApeManualMaterialJsBind.h"
#include "ApePbsPassJsBind.h"
#include "ApeManualPassJsBind.h"

#include <v8.h>
#include <uv.h>
#include <nan.h>

static int thread_id() {
#ifdef _WIN32
	return GetCurrentThreadId();
#elif __APPLE__
	return syscall(SYS_thread_selfid);
#elif __linux__
	// linux
#elif __unix__ // all unices not caught above
	// Unix
#elif defined(_POSIX_VERSION)
	return syscall(__NR_gettid);
#else
	return -1;
#endif
}

typedef v8::Persistent<v8::Function> v8PersistentFunction;
//typedef v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> v8PersistentFunction;

class AsyncFunc {
	struct payload {
		v8PersistentFunction* func;
		const char* data;
	};
public:
	AsyncFunc(v8PersistentFunction& function) : function(function) {
		handle = (uv_async_t*)malloc(sizeof(uv_async_t));
		uv_async_init(uv_default_loop(), handle, AsyncFunc::doCallback);
	};
	~AsyncFunc() {
		uv_close((uv_handle_t*)handle, close_cb);
	};
	void notify(const std::string& s) {
		notify(s.c_str());
	};
	void notify(const char* s) {
		struct payload* p = new payload();
		p->func = &function;
		p->data = s;
		handle->data = (void *)p;
		uv_async_send(handle);
	};
private:
	uv_async_t * handle;
	v8PersistentFunction function;

	static void close_cb(uv_handle_t* handle) {
		free(handle);
	};

	static void doCallback(uv_async_t* handle) {
		v8::Isolate* isolate = v8::Isolate::GetCurrent();
		v8::Local<v8::Context> context = v8::Context::New(isolate);
		const unsigned argc = 1;
		struct payload* p = (struct payload*)handle->data;
		v8::Local<v8::Value> argv[1];
		argv[0] = Nan::Null();
		/*v8::Handle<v8::Value> argv[argc] = {
			v8::Local<v8::Value>::New(v8::Number::New(v8::Isolate::GetCurrent(), 69))
		};*/
		v8::TryCatch try_catch;
		fprintf(stderr, "receiving message (thread::%d) ->\n", thread_id());
		//v8::Persistent<v8::Function>New(isolate, p->func)->Call(isolate->GetCurrentContext()->Global(), argc, argv);

		//v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> persistent(isolate, value);

		v8::Local<v8::Function> localFunction = v8::Local<v8::Function>::New(isolate, (*p->func));
		localFunction->Call(isolate->GetCurrentContext()->Global(), argc, argv);

		//v8::Local<v8::Function>::New(isolate, p->func);
		//(*p->func).Call(argc, argv);
		//(*p->func)->Call(isolate->GetCurrentContext->Global(), argc, argv);
		delete p;
		if (try_catch.HasCaught()) {
			node::FatalException(try_catch);
		}
	};
};

#ifdef NBIND_CLASS

class JsBindManager
{
	enum ErrorType
	{
		DYN_CAST_FAILED,
		NULLPTR
	};

	std::map<ErrorType, std::string> mErrorMap;

public:
	JsBindManager()
	{
		LOG_FUNC_ENTER()
		mpScene = Ape::IScene::getSingletonPtr();
		mpEventManager = Ape::IEventManager::getSingletonPtr();
		mpEventManager->connectEvent(Ape::Event::Group::NODE, std::bind(&JsBindManager::eventCallBack, this, std::placeholders::_1));
		mpSystemConfig = Ape::ISystemConfig::getSingletonPtr();
		mErrorMap.insert(std::pair<ErrorType, std::string>(DYN_CAST_FAILED, "Dynamic cast failed!"));
		mErrorMap.insert(std::pair<ErrorType, std::string>(NULLPTR, "Return value is nullptr!"));
		LOG_FUNC_LEAVE();
	}

	void eventCallBack(const Ape::Event& event)
	{
		LOG_FUNC_ENTER();
		if (mEventMap.find(event.group) != mEventMap.end())
		{
			//mEventMap.at(event.group)((int)event.type, event.subjectName); // value
			//nbind::cbFunction* cb = mEventMap.at(event.group); // pointer
			//(*cb)((int)event.type, event.subjectName);
			//mEventMap.at(event.group)((int)event.type, event.subjectName); // reference
		}

		/*if (cb)
		{
			(*cb)((int)event.type, event.subjectName);
		}*/



		//mV8Callback.Cast<v8::Local<v8::Function>>();
		v8::Local<v8::Value> retval;
		v8::Local<v8::Value> argv[1];
		argv[0] = Nan::Null();
		/*mV8CopyCallback.Get(mV8Isolate)->Call(retval, 0, argv);*/

		//mV8Callback.Get(mV8Isolate)->Call(val, 0);

		LOG(LOG_TYPE_DEBUG, "before call");
		nanCb.Call(0, argv);

		LOG_FUNC_LEAVE();
	}

	//void connectEvent(int group, nbind::cbFunction& cb)
	//{
	//	LOG_FUNC_ENTER();
	//	//mEventMap.insert(std::pair<int, nbind::cbFunction>(group, cb)); // value
	//	//mEventMap.insert(std::pair<int, nbind::cbFunction*>(group, &cb)); // pointer
	//	mEventMap.insert(std::pair<int, nbind::cbFunction&>(group, cb)); // reference
	//	LOG(LOG_TYPE_DEBUG, "event connected for group: " << group);

	//	// fire callback
	//	mEventMap.at(group)(0, "test"); // reference

	//	LOG_FUNC_LEAVE();
	//}

	void connectEvent(int group, nbind::cbFunction& pcb)
	{
		LOG_FUNC_ENTER();

		

		//mCb[0] = arr.at(0);
		//mCb.swap(arr);

		//cb = new nbind::cbFunction(pcb);

		//cb = new Nan::Callback(info[3].As<v8::Function>())

		//Nan::Persistent<v8::Object> cbRef = pcb.getJsFunction();

		//mV8Isolate = pcb.getJsFunction()->GetIsolate();
		//LOG(LOG_TYPE_DEBUG, "mV8Isolate: " << mV8Isolate);

		//mV8Callback.Reset(mV8Isolate, pcb.getJsFunction());

		// call
		//v8::Local<v8::Value> retval;
		//v8::Local<v8::Value> argv[1];
		//argv[0] = Nan::Null();
		//mV8CopyCallback.Get(mV8Isolate)->Call(retval, 0, argv);

		nanCb.Reset(pcb.getJsFunction());

		LOG_FUNC_LEAVE();
	}

	void start(std::string configFolderPath)
	{
		LOG_FUNC_ENTER();
		Ape::System::Start(configFolderPath.c_str(), true);
		LOG_FUNC_LEAVE();
	}

	void stop()
	{
		LOG_FUNC_ENTER();
		Ape::System::Stop();
		LOG_FUNC_LEAVE();
	}

	NodeJsPtr createNode(std::string name)
	{
		LOG_FUNC_ENTER();
		LOG_FUNC_LEAVE();
		return NodeJsPtr(mpScene->createNode(name));
	}

	bool getNode(std::string name, nbind::cbFunction& done)
	{
		LOG_FUNC_ENTER();
		bool success = false;
		auto entityWeakPtr = mpScene->getNode(name);
		if (auto entity = entityWeakPtr.lock())
		{
			if (auto node = std::dynamic_pointer_cast<Ape::INode>(entity))
			{
				success = true;
				done(!success, NodeJsPtr(entityWeakPtr));
			}
			else
			{
				success = false;
				done(!success, mErrorMap[ErrorType::DYN_CAST_FAILED]);
			}
		}
		else
		{
			success = false;
			done(!success, mErrorMap[ErrorType::NULLPTR]);
		}
		LOG_FUNC_LEAVE();
		return success;
	}

	bool getUserNode(nbind::cbFunction& done)
	{
		LOG_FUNC_ENTER();
		bool success = false;
		auto nodeWeakPtr = mpScene->getNode(mpSystemConfig->getSceneSessionConfig().generatedUniqueUserNodeName);
		if (auto node = nodeWeakPtr.lock())
		{
			success = true;
			done(!success, NodeJsPtr(nodeWeakPtr));
		}
		else
		{
			success = false;
			done(!success, mErrorMap[ErrorType::NULLPTR]);
		}
		LOG_FUNC_LEAVE();
		return success;
	}

	LightJsPtr createLight(std::string name)
	{
		LOG_FUNC_ENTER();
		LOG_FUNC_LEAVE();
		return LightJsPtr(mpScene->createEntity(name, Ape::Entity::LIGHT));
	}

	bool getLight(std::string name, nbind::cbFunction& done)
	{
		LOG_FUNC_ENTER();
		bool success = false;
		auto entityWeakPtr = mpScene->getEntity(name);
		if (auto entity = entityWeakPtr.lock())
		{
			if (auto textGeometry = std::dynamic_pointer_cast<Ape::ILight>(entity))
			{
				success = true;
				done(!success, LightJsPtr(entityWeakPtr));
			}
			else
			{
				success = false;
				done(!success, mErrorMap[ErrorType::DYN_CAST_FAILED]);
			}
		}
		else
		{
			success = false;
			done(!success, mErrorMap[ErrorType::NULLPTR]);
		}
		LOG_FUNC_LEAVE();
		return success;
	}

	TextJsPtr createText(std::string name)
	{
		LOG_FUNC_ENTER();
		LOG_FUNC_LEAVE();
		return TextJsPtr(mpScene->createEntity(name, Ape::Entity::GEOMETRY_TEXT));
	}

	bool getText(std::string name, nbind::cbFunction& done)
	{
		LOG_FUNC_ENTER();
		bool success = false;
		auto entityWeakPtr = mpScene->getEntity(name);
		if (auto entity = entityWeakPtr.lock())
		{
			if (auto textGeometry = std::dynamic_pointer_cast<Ape::ITextGeometry>(entity))
			{
				success = true;
				done(!success, TextJsPtr(entityWeakPtr));
			}
			else
			{
				success = false;
				done(!success, mErrorMap[ErrorType::DYN_CAST_FAILED]);
			}
		}
		else
		{
			success = false;
			done(!success, mErrorMap[ErrorType::NULLPTR]);
		}
		LOG_FUNC_LEAVE();
		return success;
	}

	IndexedFaceSetJsPtr createIndexedFaceSet(std::string name)
	{
		LOG_FUNC_ENTER();
		LOG_FUNC_LEAVE();
		return IndexedFaceSetJsPtr(mpScene->createEntity(name, Ape::Entity::GEOMETRY_INDEXEDFACESET));
	}

	bool getIndexedFaceSet(std::string name, nbind::cbFunction& done)
	{
		LOG_FUNC_ENTER();
		bool success = false;
		auto entityWeakPtr = mpScene->getEntity(name);
		if (auto entity = entityWeakPtr.lock())
		{
			if (auto indexedFaceSet = std::dynamic_pointer_cast<Ape::IIndexedFaceSetGeometry>(entity))
			{
				success = true;
				done(!success, IndexedFaceSetJsPtr(entityWeakPtr));
			}
			else
			{
				success = false;
				done(!success, mErrorMap[ErrorType::DYN_CAST_FAILED]);
			}
		}
		else
		{
			success = false;
			done(!success, mErrorMap[ErrorType::NULLPTR]);
		}
		LOG_FUNC_LEAVE();
		return success;
	}

	IndexedLineSetJsPtr createIndexedLineSet(std::string name)
	{
		LOG_FUNC_ENTER();
		LOG_FUNC_LEAVE();
		return IndexedLineSetJsPtr(mpScene->createEntity(name, Ape::Entity::GEOMETRY_INDEXEDLINESET));
	}

	bool getIndexedLineSet(std::string name, nbind::cbFunction& done)
	{
		LOG_FUNC_ENTER();
		bool success = false;
		auto entityWeakPtr = mpScene->getEntity(name);
		if (auto entity = entityWeakPtr.lock())
		{
			if (auto indexedLineSet = std::dynamic_pointer_cast<Ape::IIndexedLineSetGeometry>(entity))
			{
				success = true;
				done(!success, IndexedLineSetJsPtr(entityWeakPtr));
			}
			else
			{
				success = false;
				done(!success, mErrorMap[ErrorType::DYN_CAST_FAILED]);
			}
		}
		else
		{
			success = false;
			done(!success, mErrorMap[ErrorType::NULLPTR]);
		}
		LOG_FUNC_LEAVE();
		return success;
	}

	BoxJsPtr createBox(std::string name)
	{
		LOG_FUNC_ENTER();
		LOG_FUNC_LEAVE();
		return BoxJsPtr(mpScene->createEntity(name, Ape::Entity::GEOMETRY_BOX));
	}

	bool getBox(std::string name, nbind::cbFunction& done)
	{
		LOG_FUNC_ENTER();
		bool success = false;
		auto entityWeakPtr = mpScene->getEntity(name);
		if (auto entity = entityWeakPtr.lock())
		{
			if (auto box = std::dynamic_pointer_cast<Ape::IBoxGeometry>(entity))
			{
				success = true;
				done(!success, BoxJsPtr(entityWeakPtr));
			}
			else
			{
				success = false;
				done(!success, mErrorMap[ErrorType::DYN_CAST_FAILED]);
			}
		}
		else
		{
			success = false;
			done(!success, mErrorMap[ErrorType::NULLPTR]);
		}
		LOG_FUNC_LEAVE();
		return success;
	}

	FileGeometryJsPtr createFileGeometry(std::string name)
	{
		LOG_FUNC_ENTER();
		LOG_FUNC_LEAVE();
		return FileGeometryJsPtr(mpScene->createEntity(name, Ape::Entity::GEOMETRY_FILE));
	}

	bool getFileGeometry(std::string name, nbind::cbFunction& done)
	{
		LOG_FUNC_ENTER();
		bool success = false;
		auto entityWeakPtr = mpScene->getEntity(name);
		if (auto entity = entityWeakPtr.lock())
		{
			if (auto box = std::dynamic_pointer_cast<Ape::IFileGeometry>(entity))
			{
				success = true;
				done(!success, FileGeometryJsPtr(entityWeakPtr));
			}
			else
			{
				success = false;
				done(!success, mErrorMap[ErrorType::DYN_CAST_FAILED]);
			}
		}
		else
		{
			success = false;
			done(!success, mErrorMap[ErrorType::NULLPTR]);
		}
		LOG_FUNC_LEAVE();
		return success;
	}

	ManualMaterialJsPtr createManualMaterial(std::string name)
	{
		LOG_FUNC_ENTER();
		LOG_FUNC_LEAVE();
		return ManualMaterialJsPtr(mpScene->createEntity(name, Ape::Entity::MATERIAL_MANUAL));
	}

	bool getManualMaterial(std::string name, nbind::cbFunction& done)
	{
		LOG_FUNC_ENTER();
		bool success = false;
		auto entityWeakPtr = mpScene->getEntity(name);
		if (auto entity = entityWeakPtr.lock())
		{
			if (auto manualMaterial = std::dynamic_pointer_cast<Ape::IManualMaterial>(entity))
			{
				success = true;
				done(!success, ManualMaterialJsPtr(entityWeakPtr));
			}
			else
			{
				success = false;
				done(!success, mErrorMap[ErrorType::DYN_CAST_FAILED]);
			}
		}
		else
		{
			success = false;
			done(!success, mErrorMap[ErrorType::NULLPTR]);
		}
		LOG_FUNC_LEAVE();
		return success;
	}

	PbsPassJsPtr createPbsPass(std::string name)
	{
		LOG_FUNC_ENTER();
		LOG_FUNC_LEAVE();
		return PbsPassJsPtr(mpScene->createEntity(name, Ape::Entity::PASS_PBS));
	}

	bool getPbsPass(std::string name, nbind::cbFunction& done)
	{
		LOG_FUNC_ENTER();
		bool success = false;
		auto entityWeakPtr = mpScene->getEntity(name);
		if (auto entity = entityWeakPtr.lock())
		{
			if (auto pbsPass = std::dynamic_pointer_cast<Ape::IPbsPass>(entity))
			{
				success = true;
				done(!success, PbsPassJsPtr(entityWeakPtr));
			}
			else
			{
				success = false;
				done(!success, mErrorMap[ErrorType::DYN_CAST_FAILED]);
			}
		}
		else
		{
			success = false;
			done(!success, mErrorMap[ErrorType::NULLPTR]);
		}
		LOG_FUNC_LEAVE();
		return success;
	}

	ManualPassJsPtr createManualPass(std::string name)
	{
		LOG_FUNC_ENTER();
		LOG_FUNC_LEAVE();
		return ManualPassJsPtr(mpScene->createEntity(name, Ape::Entity::PASS_MANUAL));
	}

	bool getManualPass(std::string name, nbind::cbFunction& done)
	{
		LOG_FUNC_ENTER();
		bool success = false;
		auto entityWeakPtr = mpScene->getEntity(name);
		if (auto entity = entityWeakPtr.lock())
		{
			if (auto ManualPass = std::dynamic_pointer_cast<Ape::IManualPass>(entity))
			{
				success = true;
				done(!success, ManualPassJsPtr(entityWeakPtr));
			}
			else
			{
				success = false;
				done(!success, mErrorMap[ErrorType::DYN_CAST_FAILED]);
			}
		}
		else
		{
			success = false;
			done(!success, mErrorMap[ErrorType::NULLPTR]);
		}
		LOG_FUNC_LEAVE();
		return success;
	}

	std::string getFolderPath()
	{
		LOG_FUNC_ENTER();
		LOG_FUNC_LEAVE();
		return mpSystemConfig->getFolderPath();
	}
	
	//void On(const v8::FunctionCallbackInfo<v8::Value>& cbargs)
	//{
	//	v8::Isolate* isolate = v8::Isolate::GetCurrent();
	//	v8::HandleScope scope(isolate);
	//	if (cbargs.Length() < 2 || !cbargs[0]->IsString() || !cbargs[1]->IsFunction()) {
	//		isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "error string here")));
	//		//isolate->ThrowException(v8::String::NewFromUtf8(isolate, "error string here"));
	//	}
	//	v8::String::Utf8Value param1(cbargs[0]->ToString());
	//	std::string key = std::string(*param1);

	//	//v8::Function

	//	v8::Persistent<v8::Function>::New(isolate, cbargs[1]);
	//	v8PersistentFunction::Cast(v8::Local<v8::Function>::Cast(cbargs[1]));
	//	v8PersistentFunction cb = v8PersistentFunction::New(isolate, *v8::Local<v8::Function>::Cast(cbargs[1]));
	//	AsyncFunc* af = new AsyncFunc(cb);
	//	pool[key] = af;
	//	//return scope;
	//	//return scope.Close(v8::Undefined());
	//}

	//NAN_METHOD(CalculateAsync) {
	//	int points = Nan::To<int>(info[0]).FromJust();
	//	Nan::Callback *callback = new Nan::Callback(Nan::To<v8::Local<v8::Function>>(info[1]).ToLocalChecked());

	//	AsyncQueueWorker(new PiWorker(callback, points));
	//}

private:
	Ape::IScene* mpScene;
	Ape::IEventManager* mpEventManager;
	Ape::ISystemConfig* mpSystemConfig;
	std::map<int, nbind::cbFunction&> mEventMap;
	nbind::cbFunction* cb;
	v8::Persistent<v8::Function> mV8Callback;
	v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> mV8CopyCallback;

	std::map<int, v8::Persistent<v8::Function>> mEventMapPers;
	std::map<std::string, AsyncFunc*> pool;

	Nan::Callback nanCb;

	v8::Isolate* mV8Isolate;
};

NBIND_CLASS(JsBindManager)
{
	construct<>();

	method(connectEvent);

	method(start);
	method(stop);

	method(createNode);
	method(getNode);
	method(getUserNode);

	method(createLight);
	method(getLight);

	method(createText);
	method(getText);

	method(createIndexedFaceSet);
	method(getIndexedFaceSet);

	method(createBox);
	method(getBox);

	method(createFileGeometry);
	method(getFileGeometry);

	method(createIndexedLineSet);
	method(getIndexedLineSet);

	method(createManualMaterial);
	method(getManualMaterial);

	method(createPbsPass);
	method(getPbsPass);

	method(createManualPass);
	method(getManualPass);

	method(getFolderPath);
}

#endif

#endif
