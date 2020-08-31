#include "transport.hh"
#include "audioengine.hh"
#include "events.hh"

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
	                            InstanceAccessor("isPlaying", &Transport::GetIsPlaying, nullptr),
	                            InstanceAccessor("timingInfo", &Transport::GetTimingInfo, nullptr),
	                            InstanceAccessor("totalRuntimeSamples", &Transport::GetTotalRuntimeSamples, nullptr),
	                            InstanceMethod<&Transport::JS_start>("start"),
	                            InstanceMethod<&Transport::JS_stop>("stop"),
	                            InstanceMethod<&Transport::JS_setTimeInSamples>("setTimeInSamples"),
	                        }));

	exports.Set(Napi::String::New(env, "setActiveTransport"),
	            Napi::Function::New(env, setActiveTransport));

	return exports;
}

Transport::Transport(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Transport>(info) {
	// Napi::Env env = info.Env();
	resetTimingInfo();
	timingInfo.tempo = 120;
	timingInfo.timeSigNumerator = 4;
	timingInfo.timeSigDenominator = 4;
	timingInfo.samplerate = 0;
}

Napi::Value Transport::GetBPM(const Napi::CallbackInfo &info) {
	return Napi::Number::New(info.Env(), this->timingInfo.tempo);
}

void Transport::SetBPM(const Napi::CallbackInfo &info, const Napi::Value &value) {
	this->timingInfo.tempo = value.As<Napi::Number>().Int32Value();
}

Napi::Value Transport::GetMasterTrack(const Napi::CallbackInfo &info) {
	return this->masterTrackRef.Value();
}

void Transport::SetMasterTrack(const Napi::CallbackInfo &info, const Napi::Value &value) {
	Napi::Object obj = value.As<Napi::Object>();
	Track *track = Track::Unwrap(obj);
	track->setTransport(this);
	this->masterTrack = track;
	if (!this->masterTrackRef.IsEmpty())
		this->masterTrackRef.Unref();
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

Napi::Value Transport::GetIsPlaying(const Napi::CallbackInfo &info) {
	return Napi::Boolean::New(info.Env(), this->isPlaying);
}

Napi::Value Transport::GetTotalRuntimeSamples(const Napi::CallbackInfo &info) {
	return Napi::Number::New(info.Env(), this->runtimeSamples);
}

Napi::Value Transport::GetTimingInfo(const Napi::CallbackInfo &info) {
	Napi::Object obj = Napi::Object::New(info.Env());
	obj.Set("projectTimeSamples", this->timingInfo.projectTimeSamples);
	obj.Set("projectTimeMusic", this->timingInfo.projectTimeMusic);
	obj.Set("projectTimeMusicBars", this->timingInfo.projectTimeMusicBars);
	obj.Set("barPositionMusic", this->timingInfo.barPositionMusic);
	obj.Set("tempo", this->timingInfo.tempo);
	obj.Set("timeSigDenominator", this->timingInfo.timeSigDenominator);
	obj.Set("timeSigNumerator", this->timingInfo.timeSigNumerator);
	obj.Set("samplerate", this->timingInfo.samplerate);
	return obj;
}

Napi::Value Transport::JS_start(const Napi::CallbackInfo &info){
	this->start();
	return info.Env().Undefined();
}

Napi::Value Transport::JS_stop(const Napi::CallbackInfo &info) {
	this->stop();
	return info.Env().Undefined();
}

Napi::Value Transport::JS_setTimeInSamples(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();

	if (info.Length() < 1) {
		Napi::TypeError::New(env, "WRONG_ARGUMENTS_COUNT").ThrowAsJavaScriptException();
		return info.Env().Undefined();
	}

	if (!info[0].IsNumber()) {
		Napi::TypeError::New(env, "WRONG_ARGUMENTS").ThrowAsJavaScriptException();
		return info.Env().Undefined();
	}

	this->setTimeInSamples(info[0].As<Napi::Number>().Int64Value());
	return info.Env().Undefined();
}

void Transport::setTimeInSamples(unsigned long long s) {
	if (s != 0) {
		this->timingInfo.projectTimeSamples = s;

		int samplerate = dac->getStreamSampleRate();
		double sampleduration = 1.0 / samplerate;
		double bps = 60.0 / activeTransport->timingInfo.tempo;

		timingInfo.projectTimeMusic = timingInfo.projectTimeSamples * sampleduration / bps;
		const double quarterNotesPerBar =
		    timingInfo.timeSigNumerator / (timingInfo.timeSigDenominator / 4);
		// ti.projectTimeMusicBars =
		//     (ti.timeSigDenominator / 4 * ti.projectTimeMusic) / ti.timeSigNumerator;
		timingInfo.projectTimeMusicBars = timingInfo.projectTimeMusic / quarterNotesPerBar;
		timingInfo.barPositionMusic =
		    timingInfo.projectTimeMusic -
		    (timingInfo.projectTimeMusicBars - int(timingInfo.projectTimeMusicBars)) *
		        quarterNotesPerBar;
	} else {
		resetTimingInfo();
	}
}

void Transport::start() {
	this->isPlaying = true;
	sendEventNonBlocking("transportIsPlaying", [](Napi::Env env) {
		return Napi::Boolean::New(env, activeTransport->isPlaying);
	});
}

void Transport::stop() {
	if (this->isPlaying == false) {
		resetTimingInfo();

		TimingInfo ti = this->timingInfo;
		sendEventNonBlocking("transportUpdated", [ti](Napi::Env env) {
			Napi::Object obj = Napi::Object::New(env);
			obj.Set("projectTimeSamples", ti.projectTimeSamples);
			obj.Set("projectTimeMusic", ti.projectTimeMusic);
			obj.Set("projectTimeMusicBars", ti.projectTimeMusicBars);
			return obj;
		});
	} else {
		this->isPlaying = false;
		sendEventNonBlocking("transportIsPlaying", [](Napi::Env env) {
			return Napi::Boolean::New(env, activeTransport->isPlaying);
		});
	}
	cueTrack->recursiveRemoveOneshotTrackEvents();
	masterTrack->recursiveRemoveOneshotTrackEvents();
}

int Transport::loop(double *outputBuffer, double *inputBuffer, unsigned int nBufferFrames,
                    double streamTime) {

	// for (unsigned int i = 0; i < nBufferFrames; i++) {
	// 	const double from = streamTime + i * sampleduration;
	// 	const double to = streamTime + (i + 1) * sampleduration;
	// 	if (fmod(from, bps) > fmod(to, bps)) {
	// 		//std::cout << "beat" << std::endl;
	// 	}
	// }

	if (isPlaying) {
		setTimeInSamples(timingInfo.projectTimeSamples);
	}

	//std::cout << ti.projectTimeMusic << " / " << ti.barPositionMusic << std::endl;

	memset(outputBuffer, 0,
	       sizeof(double) * nBufferFrames * 2); // TODO: hardcoded channel count

	// for (sf_count_t frame = 0; frame < nBufferFrames; frame++) {
	// 	for (int channel = 0; channel < 2; channel++) { // TODO: hardcoded channel count
	// 	}
	// }

	timingInfo.samplerate = dac->getStreamSampleRate();

	TimingInfo ti = this->timingInfo;
	if (masterTrack) {
		masterTrack->process(outputBuffer, inputBuffer, nBufferFrames, ti);
	}
	if (cueTrack) {
		TimingInfo cueTi = this->timingInfo;
		cueTi.projectTimeSamples = this->runtimeSamples;
		cueTrack->process(outputBuffer, inputBuffer, nBufferFrames, cueTi);
	}

	if (this->isPlaying) {
		sendEventNonBlocking("transportUpdated", [ti](Napi::Env env) {
			Napi::Object obj = Napi::Object::New(env);
			obj.Set("projectTimeSamples", ti.projectTimeSamples);
			obj.Set("projectTimeMusic", ti.projectTimeMusic);
			obj.Set("projectTimeMusicBars", ti.projectTimeMusicBars);
			return obj;
		});
	}

	timingInfo.projectTimeSamples += +nBufferFrames;
	runtimeSamples += nBufferFrames;

	return 0;
}
