#include "events.hh"

static bool eventCallbackSet = false;
static Napi::ThreadSafeFunction eventCallbackTsfn;

Napi::Value setEventsCallback(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();

	if (info.Length() < 1) {
		throw Napi::TypeError::New(env, "Expected one arguments");
	} else if (!info[0].IsFunction()) {
		throw Napi::TypeError::New(env, "Expected first arg to be function");
	}

	eventCallbackTsfn = Napi::ThreadSafeFunction::New(
	    env,
	    info[0].As<Napi::Function>(),  // JavaScript function called asynchronously
	    "eventCallbackTsfn",           // Name
	    0,                             // Unlimited queue
	    1                              // Only one thread will use this initially
	);

	eventCallbackSet = true;

	return env.Undefined();
}

void sendEventNonBlocking(const char *eventName, std::function<Napi::Value(Napi::Env)> eventFn) {
	if (!eventCallbackSet)
		return;
	eventCallbackTsfn.NonBlockingCall(
	    [eventName, eventFn](Napi::Env env, Napi::Function jsCallback) {
		    jsCallback.Call({Napi::String::New(env, eventName), eventFn(env)});
	    });
}

void sendEventNonBlocking(const char *eventName) {
	if (!eventCallbackSet)
		return;
	eventCallbackTsfn.NonBlockingCall(
	    [eventName](Napi::Env env, Napi::Function jsCallback) {
		    jsCallback.Call({Napi::String::New(env, eventName)});
	    });
}

void sendEventBlocking(const char *eventName, std::function<Napi::Value(Napi::Env)> eventFn) {
	if (!eventCallbackSet)
		return;
	eventCallbackTsfn.BlockingCall([eventName, eventFn](Napi::Env env, Napi::Function jsCallback) {
		jsCallback.Call({Napi::String::New(env, eventName), eventFn(env)});
	});
}

void sendEventBlocking(const char *eventName) {
	if (!eventCallbackSet)
		return;
	eventCallbackTsfn.BlockingCall([eventName](Napi::Env env, Napi::Function jsCallback) {
		jsCallback.Call({Napi::String::New(env, eventName)});
	});
}
