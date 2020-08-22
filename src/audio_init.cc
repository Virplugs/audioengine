#include <iostream>
#include <vector>

#include "events.hh"
#include "audioengine.hh"

#include <RtAudio.h>
#include <napi.h>

int saw(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime,
        RtAudioStreamStatus status, void *userData) {
	/*unsigned int i, j;
	double *buffer = (double *)outputBuffer;
	double *lastValues = (double *)userData;
	if (status)
		std::cout << "Stream underflow detected!" << std::endl;
	// Write interleaved audio data.
	for (i = 0; i < nBufferFrames; i++) {
		for (j = 0; j < 2; j++) {
			*buffer++ = lastValues[j];
			lastValues[j] += 0.005 * (j + 1 + (j * 0.1));
			if (lastValues[j] >= 1.0)
				lastValues[j] -= 2.0;
		}
	}*/
	return 0;
}

RtAudio *dac = nullptr;
double data[2];

Napi::Value requestLatencyInfo(const Napi::CallbackInfo &info) {
	printf("requestLatencyInfo\n");
	const Napi::Env env = info.Env();

	if (dac == nullptr) {
		Napi::Error::New(env, "Audio Engine not started.").ThrowAsJavaScriptException();
		return env.Undefined();
	}

	sendEventNonBlocking("latencyUpdated", [](Napi::Env env) {
		Napi::Object obj = Napi::Object::New(env);
		long actualSamplerate = dac->getStreamSampleRate();
		obj.Set("input", (float)dac->getStreamInputLatency() / actualSamplerate * 1000.0);
		obj.Set("output", (float)dac->getStreamOutputLatency() / actualSamplerate * 1000.0);
		obj.Set("total", (float)dac->getStreamLatency() / actualSamplerate * 1000.0);
		obj.Set("samplerate", actualSamplerate);
		return obj;
	});

	return env.Undefined();
}

Napi::Value startAudioEngine(const Napi::CallbackInfo &info) {
	printf("startAudioEngine\n");
	const Napi::Env env = info.Env();

	if (info.Length() < 5) {
		Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
		return Napi::Boolean::New(env, false);
	}

	if (!info[0].IsString() || !info[1].IsNumber() || !info[2].IsNumber() || !info[3].IsNumber() ||
	    !info[4].IsNumber()) {
		Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
		return Napi::Boolean::New(env, false);
	}

	if (dac != nullptr) {
		Napi::Error::New(env, "Audio Engine already started. Stop first.").ThrowAsJavaScriptException();
		return Napi::Boolean::New(env, false);
	}

	RtAudio::Api api = RtAudio::getCompiledApiByName(info[0].As<Napi::String>().Utf8Value());

	if (api == RtAudio::UNSPECIFIED) {
		Napi::Error::New(env, "Audio API not available").ThrowAsJavaScriptException();
		return Napi::Boolean::New(env, false);
	}

	dac = new RtAudio(api);
	RtAudio::StreamParameters outputParameters, inputParameters;
	outputParameters.deviceId = info[2].As<Napi::Number>().Int32Value();
	outputParameters.nChannels = dac->getDeviceInfo(outputParameters.deviceId).outputChannels;
	outputParameters.firstChannel = 0;
	inputParameters.deviceId = info[1].As<Napi::Number>().Int32Value();
	inputParameters.nChannels = dac->getDeviceInfo(inputParameters.deviceId).inputChannels;
	inputParameters.firstChannel = 0;
	unsigned int sampleRate = info[3].As<Napi::Number>().Int32Value();
	unsigned int bufferFrames = info[4].As<Napi::Number>().Int32Value();

	try {
		dac->openStream(&outputParameters, &inputParameters, RTAUDIO_FLOAT64, sampleRate,
		                &bufferFrames, &saw, (void *)&data);
		dac->startStream();

		requestLatencyInfo(info);
	} catch (RtAudioError &e) {
		Napi::Error::New(env, e.getMessage().c_str()).ThrowAsJavaScriptException();
		delete dac;
		dac = nullptr;
		return Napi::Boolean::New(env, false);
	}

	return Napi::Boolean::New(env, true);
}

Napi::Value stopAudioEngine(const Napi::CallbackInfo &info) {
	printf("stopAudioEngine\n");
	if (dac != nullptr) {
		try {
			// Stop the stream
			dac->abortStream();
		} catch (RtAudioError &e) {
			Napi::Error::New(info.Env(), e.getMessage().c_str()).ThrowAsJavaScriptException();
			delete dac;
			dac = nullptr;
			return Napi::Boolean::New(info.Env(), false);
		}
		if (dac->isStreamOpen())
			dac->closeStream();
		delete dac;
		dac = nullptr;
	}
	return Napi::Boolean::New(info.Env(), true);
}

