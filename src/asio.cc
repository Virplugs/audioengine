#include <iostream>
#include <vector>

#include "audioengine.hh"

#include <RtAudio.h>
#include <napi.h>
#include <asio.h>

Napi::Value openAsioControlPanel(const Napi::CallbackInfo &info) {
	const Napi::Env env = info.Env();

	if (dac == nullptr) {
		Napi::Error::New(env, "Audio Engine not started.").ThrowAsJavaScriptException();
		return Napi::Boolean::New(env, false);
	}

	if (dac->getCurrentApi() != RtAudio::WINDOWS_ASIO) {
		Napi::Error::New(env, "No ASIO driver loaded currently.").ThrowAsJavaScriptException();
		return Napi::Boolean::New(env, false);
	}

	ASIOControlPanel();

	return Napi::Boolean::New(env, true);
}
