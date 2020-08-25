#include "transport.hh"
#include "audioengine.hh"

#include <cmath>

Napi::Value setActiveTransport(const Napi::CallbackInfo &info);

Transport *activeTransport = nullptr;

Napi::Object Transport::Init(Napi::Env env, Napi::Object exports) {
	exports.Set("Transport", DefineClass(env, "Transport",
		{
			InstanceAccessor("bpm", &Transport::GetBPM, &Transport::SetBPM),
			InstanceAccessor("tracks", &Transport::GetTracks, &Transport::SetTracks),
		}));

	exports.Set(Napi::String::New(env, "setActiveTransport"),
	            Napi::Function::New(env, setActiveTransport));

	return exports;
}

Transport::Transport(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Transport>(info) {
	//Napi::Env env = info.Env();
}

Napi::Value Transport::GetBPM(const Napi::CallbackInfo &info) {
	return Napi::Number::New(info.Env(), this->bpm);
}

void Transport::SetBPM(const Napi::CallbackInfo &info, const Napi::Value &value) {
	this->bpm = value.As<Napi::Number>().Int32Value();
}

Napi::Value Transport::GetTracks(const Napi::CallbackInfo &info) {
	return this->tracksRef.Value();
}

void Transport::SetTracks(const Napi::CallbackInfo &info, const Napi::Value &value) {
	this->tracksRef = Napi::Reference<Napi::Array>::New(value.As<Napi::Array>(), 1);

	this->tracks.clear();
	for (unsigned int i = 0; i < this->tracksRef.Value().Length(); i++) {
		Track *track = Track::Unwrap(this->tracksRef.Value().Get(i).As<Napi::Object>());
		track->setTransport(this);
		this->tracks.push_back(track);
	}
}

void Transport::Finalize(const Napi::Env env) {
	if (activeTransport == this) activeTransport = nullptr;
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
			std::cout << "beat" << std::endl;
		}
	}

	memset(outputBuffer, 0, sizeof(double) * nBufferFrames * 2); // TODO: hardcoded channel count

	// for (sf_count_t frame = 0; frame < nBufferFrames; frame++) {
	// 	for (int channel = 0; channel < 2; channel++) { // TODO: hardcoded channel count
	// 	}
	// }

	for (const auto &track : tracks) {
		track->process(outputBuffer, inputBuffer, nBufferFrames);
	}

	frameCount += nBufferFrames;

	return 0;
}
