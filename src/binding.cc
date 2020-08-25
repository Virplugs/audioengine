#include <napi.h>

#include "track.hh"
#include "audioevent.hh"
#include "transport.hh"

Napi::Object getDeviceInfo(const Napi::CallbackInfo &info);
Napi::Value readAudioFileInfo(const Napi::CallbackInfo &info);
Napi::Value readAudioFileWaveform(const Napi::CallbackInfo &info);
Napi::Value startAudioEngine(const Napi::CallbackInfo &info);
Napi::Value stopAudioEngine(const Napi::CallbackInfo &info);
Napi::Value openAsioControlPanel(const Napi::CallbackInfo &info);
Napi::Value setEventsCallback(const Napi::CallbackInfo &info);
Napi::Value requestLatencyInfo(const Napi::CallbackInfo &info);

Napi::Object Init(Napi::Env env, Napi::Object exports) {
	exports.Set(Napi::String::New(env, "getDeviceInfo"),
	            Napi::Function::New(env, getDeviceInfo));
	exports.Set(Napi::String::New(env, "readAudioFileInfo"),
	            Napi::Function::New(env, readAudioFileInfo));
	exports.Set(Napi::String::New(env, "readAudioFileWaveform"),
	            Napi::Function::New(env, readAudioFileWaveform));
	exports.Set(Napi::String::New(env, "startAudioEngine"),
	            Napi::Function::New(env, startAudioEngine));
	exports.Set(Napi::String::New(env, "stopAudioEngine"),
	            Napi::Function::New(env, stopAudioEngine));
	exports.Set(Napi::String::New(env, "openAsioControlPanel"),
	            Napi::Function::New(env, openAsioControlPanel));
	exports.Set(Napi::String::New(env, "setEventsCallback"),
	            Napi::Function::New(env, setEventsCallback));
	exports.Set(Napi::String::New(env, "requestLatencyInfo"),
	            Napi::Function::New(env, requestLatencyInfo));

	Track::Init(env, exports);
	AudioEvent::Init(env, exports);
	Transport::Init(env, exports);

	return exports;
}

NODE_API_MODULE(audioengine, Init)
