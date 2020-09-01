#include "audioevent.hh"
#include "track.hh"

Napi::Object AudioEvent::Init(Napi::Env env, Napi::Object exports) {
	exports.Set(
	    "NativeAudioEvent",
	    DefineClass(env, "NativeAudioEvent",
	                {
	                    InstanceAccessor("name", &AudioEvent::GetName, &AudioEvent::SetName),
	                    InstanceAccessor("lastFrameOffset", &AudioEvent::GetLastFrameOffset, nullptr),
	                    InstanceAccessor("duration", &AudioEvent::GetDuration, nullptr),
	                    InstanceAccessor("totalFrames", &AudioEvent::GetTotalFrames, nullptr),
	                }));

	return exports;
}

AudioEvent::AudioEvent(const Napi::CallbackInfo &info) : Napi::ObjectWrap<AudioEvent>(info) {
	Napi::Env env = info.Env();

	if (info.Length() < 2) {
		Napi::TypeError::New(env, "WRONG_ARGUMENTS_COUNT").ThrowAsJavaScriptException();
		return;
	}

	if (!info[0].IsString() && !info[1].IsString()) {
		Napi::TypeError::New(env, "WRONG_ARGUMENTS").ThrowAsJavaScriptException();
		return;
	}

	this->name = info[0].As<Napi::String>().Utf8Value();
	this->filename = info[1].As<Napi::String>().Utf8Value();

	memset(&sfinfo, 0, sizeof(sfinfo));
	if ((infile = sf_open(filename.c_str(), SFM_READ, &sfinfo)) == NULL) {
		Napi::TypeError::New(env, sf_strerror(NULL)).ThrowAsJavaScriptException();
	};

	this->totalFrames = sfinfo.frames;
	this->duration = (double)sfinfo.frames / (double)sfinfo.samplerate;
}

Napi::Value AudioEvent::GetName(const Napi::CallbackInfo &info) {
	return Napi::String::New(info.Env(), this->name);
}

void AudioEvent::SetName(const Napi::CallbackInfo &info, const Napi::Value &value) {
	this->name = value.As<Napi::String>().Utf8Value();
}

void AudioEvent::Finalize(const Napi::Env env) {
	sf_close(infile);
}

int AudioEvent::render(double *outputBuffer, double *inputBuffer, unsigned int nBufferFrames,
                       unsigned int nOffsetFrames) {
	sf_seek(infile, nOffsetFrames, SEEK_SET);

	float *buf = (float *)malloc(sizeof(float) * nBufferFrames * sfinfo.channels);
	sf_count_t readcount;

	if ((readcount = sf_readf_float(infile, buf, nBufferFrames)) > 0) {
		for (sf_count_t frame = 0; frame < readcount; frame++) {
			for (int channel = 0; channel < sfinfo.channels; channel++) {
				const float val = buf[frame * sfinfo.channels + channel];
				outputBuffer[frame * sfinfo.channels + channel] += val;
			}
		};
	}

	free(buf);

	this->lastFrameOffset = nOffsetFrames + nBufferFrames;

	return 0;
}

Napi::Value AudioEvent::GetLastFrameOffset(const Napi::CallbackInfo &info) {
	return Napi::Number::New(info.Env(), this->lastFrameOffset);
}

Napi::Value AudioEvent::GetDuration(const Napi::CallbackInfo &info) {
	return Napi::Number::New(info.Env(), this->duration);
}

Napi::Value AudioEvent::GetTotalFrames(const Napi::CallbackInfo &info) {
	return Napi::Number::New(info.Env(), this->totalFrames);
}
