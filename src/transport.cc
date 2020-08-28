#include "transport.hh"
#include "audioengine.hh"

#include <cmath>
#include <algorithm>

Napi::Value setActiveTransport(const Napi::CallbackInfo &info);

Transport *activeTransport = nullptr;

Napi::Object Transport::Init(Napi::Env env, Napi::Object exports) {
	exports.Set("NativeTransport",
	            DefineClass(env, "NativeTransport",
	                        {
	                            InstanceAccessor("bpm", &Transport::GetBPM, &Transport::SetBPM),
	                            InstanceAccessor("masterTrack", &Transport::GetMasterTrack,
	                                             &Transport::SetMasterTrack),
	                            InstanceAccessor("cueTrack", &Transport::GetCueTrack,
	                                             &Transport::SetCueTrack),
	                        }));

	exports.Set(Napi::String::New(env, "setActiveTransport"),
	            Napi::Function::New(env, setActiveTransport));

	return exports;
}

Transport::Transport(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Transport>(info) {
	// Napi::Env env = info.Env();
}

Napi::Value Transport::GetBPM(const Napi::CallbackInfo &info) {
	return Napi::Number::New(info.Env(), this->bpm);
}

void Transport::SetBPM(const Napi::CallbackInfo &info, const Napi::Value &value) {
	this->bpm = value.As<Napi::Number>().Int32Value();
}

Napi::Value Transport::GetMasterTrack(const Napi::CallbackInfo &info) {
	return this->masterTrackRef.Value();
}

void Transport::SetMasterTrack(const Napi::CallbackInfo &info, const Napi::Value &value) {
	Napi::Object obj = value.As<Napi::Object>();
	Track *track = Track::Unwrap(obj);
	track->setTransport(this);
	this->masterTrack = track;
	if (!this->masterTrackRef.IsEmpty()) this->masterTrackRef.Unref();
	this->masterTrackRef = Napi::Reference<Napi::Object>::New(obj, 1);
}

Napi::Value Transport::GetCueTrack(const Napi::CallbackInfo &info) {
	return this->cueTrackRef.Value();
}

void Transport::SetCueTrack(const Napi::CallbackInfo &info, const Napi::Value &value) {
	Napi::Object obj = value.As<Napi::Object>();
	Track *track = Track::Unwrap(obj);
	track->setTransport(this);
	this->cueTrack = track;
	if (!this->cueTrackRef.IsEmpty())
		this->cueTrackRef.Unref();
	this->cueTrackRef = Napi::Reference<Napi::Object>::New(obj, 1);
}

void Transport::Finalize(const Napi::Env env) {
	if (activeTransport == this)
		activeTransport = nullptr;
}

Napi::Value setActiveTransport(const Napi::CallbackInfo &info) {
	activeTransport = Transport::Unwrap(info[0].As<Napi::Object>());

	return info.Env().Undefined();
}

int Transport::loop(double *outputBuffer, double *inputBuffer, unsigned int nBufferFrames,
                    double streamTime) {
	double sampleduration = 1.0 / dac->getStreamSampleRate();
	double bps = 60.0 / activeTransport->bpm;

	for (unsigned int i = 0; i < nBufferFrames; i++) {
		const double from = streamTime + i * sampleduration;
		const double to = streamTime + (i + 1) * sampleduration;
		if (fmod(from, bps) > fmod(to, bps)) {
			//std::cout << "beat" << std::endl;
		}
	}

	memset(outputBuffer, 0,
	       sizeof(double) * nBufferFrames * 2); // TODO: hardcoded channel count

	// for (sf_count_t frame = 0; frame < nBufferFrames; frame++) {
	// 	for (int channel = 0; channel < 2; channel++) { // TODO: hardcoded channel count
	// 	}
	// }

	if (masterTrack) masterTrack->process(outputBuffer, inputBuffer, nBufferFrames);
	if (cueTrack) cueTrack->process(outputBuffer, inputBuffer, nBufferFrames);

	frameCount += nBufferFrames;

	return 0;
}
