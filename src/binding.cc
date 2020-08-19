#include <napi.h>

Napi::Object getDeviceInfo(const Napi::CallbackInfo &info);
Napi::Value readAudioFileInfo(const Napi::CallbackInfo &info);
Napi::Value readAudioFileWaveform(const Napi::CallbackInfo &info);

Napi::Object Init(Napi::Env env, Napi::Object exports) {
	exports.Set(Napi::String::New(env, "getDeviceInfo"),
	            Napi::Function::New(env, getDeviceInfo));
	exports.Set(Napi::String::New(env, "readAudioFileInfo"),
	            Napi::Function::New(env, readAudioFileInfo));
	exports.Set(Napi::String::New(env, "readAudioFileWaveform"),
	            Napi::Function::New(env, readAudioFileWaveform));
	return exports;
}

NODE_API_MODULE(audioengine, Init)
