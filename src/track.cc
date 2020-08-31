#include "audioengine.hh"
#include "track.hh"

#include <cmath>

Napi::Object Track::Init(Napi::Env env, Napi::Object exports) {
	exports.Set("NativeTrack",
	            DefineClass(env, "NativeTrack",
	                        {
	                            InstanceMethod<&Track::JS_playAudioEvent>("playAudioEvent"),
	                            InstanceMethod<&Track::JS_stopAudioEvent>("stopAudioEvent"),
	                            // InstanceMethod<&Track::SetValue>("SetValue"),
	                            InstanceAccessor("name", &Track::GetName, &Track::SetName),
	                            InstanceAccessor("inputChannels", &Track::GetInputChannels,
	                                             &Track::SetInputChannels),
	                            InstanceAccessor("outputChannels", &Track::GetOutputChannels,
	                                             &Track::SetOutputChannels),
	                            InstanceAccessor("levels", &Track::GetLevels, nullptr),
	                            InstanceAccessor("volume", &Track::GetVolume, &Track::SetVolume),

	                            InstanceAccessor("subTracks", &Track::GetSubTracks, nullptr),
	                            InstanceMethod<&Track::AddSubTrack>("addSubTrack"),
	                            InstanceMethod<&Track::RemoveSubTrack>("removeSubTrack"),

	                        }));

	return exports;
}

Track::Track(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Track>(info) {
	Napi::Env env = info.Env();

	if (info.Length() < 3) {
		Napi::TypeError::New(env, "WRONG_ARGUMENTS_COUNT").ThrowAsJavaScriptException();
		return;
	}

	if (!info[0].IsString() || !info[1].IsArray() || !info[2].IsArray()) {
		Napi::TypeError::New(env, "WRONG_ARGUMENTS").ThrowAsJavaScriptException();
		return;
	}

	this->name = info[0].As<Napi::String>().Utf8Value();

	Napi::Array inputArr = info[1].As<Napi::Array>();
	for (unsigned int i = 0; i < inputArr.Length(); i++) {
		this->inputChannels.push_back(inputArr.Get(i).As<Napi::Number>().Int32Value());
	}
	Napi::Array outputArr = info[2].As<Napi::Array>();
	for (unsigned int i = 0; i < outputArr.Length(); i++) {
		this->outputChannels.push_back(outputArr.Get(i).As<Napi::Number>().Int32Value());
	}
}

Track::~Track() {
	if (this->bufferSize > 0) {
		free(this->outputBuffer);
	}
}

void Track::Finalize(const Napi::Env env) {
	std::cout << "track finalize";
}

Napi::Value Track::GetName(const Napi::CallbackInfo &info) {
	return Napi::String::New(info.Env(), this->name);
}

void Track::SetName(const Napi::CallbackInfo &info, const Napi::Value &value) {
	this->name = value.As<Napi::String>().Utf8Value();
}

Napi::Value Track::GetVolume(const Napi::CallbackInfo &info) {
	return Napi::Number::New(info.Env(), this->volume);
}

void Track::SetVolume(const Napi::CallbackInfo &info, const Napi::Value &value) {
	this->volume = value.As<Napi::Number>().DoubleValue();
}

Napi::Value Track::GetInputChannels(const Napi::CallbackInfo &info) {
	Napi::Array arr = Napi::Array::New(info.Env(), this->inputChannels.size());

	for (unsigned int i = 0; i < inputChannels.size(); i++) {
		arr[i] = inputChannels[i];
	}

	return arr;
}

void Track::SetInputChannels(const Napi::CallbackInfo &info, const Napi::Value &value) {
	Napi::Array inputArr = value.As<Napi::Array>();
	this->inputChannels.clear();
	for (unsigned int i = 0; i < inputArr.Length(); i++) {
		this->inputChannels.push_back(inputArr.Get(i).As<Napi::Number>().Int32Value());
	}
}

Napi::Value Track::GetOutputChannels(const Napi::CallbackInfo &info) {
	Napi::Array arr = Napi::Array::New(info.Env(), this->outputChannels.size());

	for (unsigned int i = 0; i < outputChannels.size(); i++) {
		arr[i] = outputChannels[i];
	}

	return arr;
}

void Track::SetOutputChannels(const Napi::CallbackInfo &info, const Napi::Value &value) {
	Napi::Array outputArr = value.As<Napi::Array>();
	this->outputChannels.clear();
	for (unsigned int i = 0; i < outputArr.Length(); i++) {
		this->outputChannels.push_back(outputArr.Get(i).As<Napi::Number>().Int32Value());
	}
}

Napi::Value Track::JS_playAudioEvent(const Napi::CallbackInfo &info) {
	if (this->transport == nullptr) {
		Napi::TypeError::New(info.Env(), "Track has no transport linked")
		    .ThrowAsJavaScriptException();
		return info.Env().Undefined();
	}
	unsigned long long time = transport->timingInfo.projectTimeSamples;
	if (info.Length() >= 2 && info[1].IsNumber()) {
		time = info[1].As<Napi::Number>().Int64Value();
	}

	auto s = ScheduledTrackEvent(time, AudioEvent::Unwrap(info[0].As<Napi::Object>()),
	                             info[0].As<Napi::Object>(), true);
	this->scheduleTrackEvent(s);
	return info.Env().Undefined();
}

void Track::playAudioEvent(AudioEvent *e) {
	this->scheduleTrackEvent(
	    ScheduledTrackEvent(transport->timingInfo.projectTimeSamples + 256, e, true));
}

Napi::Value Track::JS_stopAudioEvent(const Napi::CallbackInfo &info) {
	this->stopAudioEvent(AudioEvent::Unwrap(info[0].As<Napi::Object>()));
	return info.Env().Undefined();
}

void Track::stopAudioEvent(AudioEvent *e) {
	ScheduledTrackEvent *currentTrackEvent = this->trackEvents;

	while (currentTrackEvent != nullptr) {
		if (currentTrackEvent->event == e) {
			if (currentTrackEvent->prev != nullptr)
				currentTrackEvent->prev->next = currentTrackEvent->next;
			else
				this->trackEvents = currentTrackEvent->next;
			if (currentTrackEvent->next != nullptr)
				currentTrackEvent->next->prev = currentTrackEvent->prev;
			if (this->trackEvents == currentTrackEvent)
				this->trackEvents = nullptr;
			ScheduledTrackEvent *old = currentTrackEvent;
			currentTrackEvent = currentTrackEvent->next;
			delete old;
		} else {
			currentTrackEvent = currentTrackEvent->next;
		}
	}
}

void Track::scheduleTrackEvent(const ScheduledTrackEvent &e) {
	if (this->trackEvents == nullptr) {
		this->trackEvents = new ScheduledTrackEvent(e);
	} else {
		ScheduledTrackEvent *lastEvent = this->trackEvents;
		while (lastEvent->next != nullptr) {
			lastEvent = lastEvent->next;
		}
		lastEvent->next = new ScheduledTrackEvent(e);
		lastEvent->next->prev = lastEvent;
	}
	this->cleanupScheduledTrackEvents();
}

void Track::cleanupScheduledTrackEvents() {
	ScheduledTrackEvent *currentTrackEvent = this->trackEvents;

	// find the first event that should be playing
	while (currentTrackEvent != nullptr) {
		if (transport->timingInfo.projectTimeSamples >
		    currentTrackEvent->time + currentTrackEvent->event->totalFrames) {
			// this is a trackevent that has been done playing
			if (currentTrackEvent->triggerOnce) {
				if (currentTrackEvent->prev != nullptr)
					currentTrackEvent->prev->next = currentTrackEvent->next;
				else
					this->trackEvents = currentTrackEvent->next;
				if (currentTrackEvent->next != nullptr)
					currentTrackEvent->next->prev = currentTrackEvent->prev;
				ScheduledTrackEvent *old = currentTrackEvent;
				currentTrackEvent = currentTrackEvent->next;
				if (this->trackEvents == old) {
					this->trackEvents = currentTrackEvent->next;
				}
				delete old;
				continue;
			}
		}

		currentTrackEvent = currentTrackEvent->next;
	}
}

void Track::recursiveRemoveOneshotTrackEvents() {
	ScheduledTrackEvent *currentTrackEvent = this->trackEvents;

	// find the first event that should be playing
	while (currentTrackEvent != nullptr) {
		if (currentTrackEvent->triggerOnce) {
			if (currentTrackEvent->prev != nullptr)
				currentTrackEvent->prev->next = currentTrackEvent->next;
			else
				this->trackEvents = currentTrackEvent->next;
			if (currentTrackEvent->next != nullptr)
				currentTrackEvent->next->prev = currentTrackEvent->prev;
			ScheduledTrackEvent *old = currentTrackEvent;
			currentTrackEvent = currentTrackEvent->next;
			if (this->trackEvents == old) {
				this->trackEvents = currentTrackEvent->next;
			}
			delete old;
		}
	}

	for (auto track : this->subTracks) {
		track->recursiveRemoveOneshotTrackEvents();
	}
}

Napi::Value Track::GetLevels(const Napi::CallbackInfo &info) {
	Napi::Array arr = Napi::Array::New(info.Env(), 2);
	arr[(uint32_t)0] = Napi::Number::New(info.Env(), levels[0]);
	arr[(uint32_t)1] = Napi::Number::New(info.Env(), levels[1]);
	return arr;
}

Napi::Value Track::AddSubTrack(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();

	if (info.Length() < 1) {
		Napi::TypeError::New(env, "WRONG_ARGUMENTS_COUNT").ThrowAsJavaScriptException();
		return env.Undefined();
	}

	if (!info[0].IsObject()) {
		Napi::TypeError::New(env, "WRONG_ARGUMENTS").ThrowAsJavaScriptException();
		return env.Undefined();
	}

	Napi::Object obj = info[0].As<Napi::Object>();
	Track *track = Track::Unwrap(obj);
	track->setTransport(this->transport);
	if (info.Length() >= 2 && info[1].IsNumber()) {
		int index = info[1].As<Napi::Number>().Int32Value();
		this->subTracks.insert(this->subTracks.begin() + index, track);
		this->subTrackRefs.insert(this->subTrackRefs.begin() + index,
		                          Napi::Reference<Napi::Object>::New(obj, 1));
	} else {
		this->subTracks.push_back(track);
		this->subTrackRefs.push_back(Napi::Reference<Napi::Object>::New(obj, 1));
	}

	return env.Undefined();
}

Napi::Value Track::RemoveSubTrack(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();

	if (info.Length() < 1) {
		Napi::TypeError::New(env, "WRONG_ARGUMENTS_COUNT").ThrowAsJavaScriptException();
		return env.Undefined();
	}

	if (!info[0].IsObject()) {
		Napi::TypeError::New(env, "WRONG_ARGUMENTS").ThrowAsJavaScriptException();
		return env.Undefined();
	}

	Napi::Object obj = info[0].As<Napi::Object>();
	Track *track = Track::Unwrap(obj);

	track->setTransport(nullptr);

	subTracks.erase(std::remove(subTracks.begin(), subTracks.end(), track), subTracks.end());

	auto refIt = std::find_if(
	    std::begin(subTrackRefs), std::end(subTrackRefs),
	    [track](const Napi::ObjectReference &ref) { return Track::Unwrap(ref.Value()) == track; });
	if (refIt != std::end(subTrackRefs)) {
		(*refIt).Unref();
		subTrackRefs.erase(refIt);
	}

	return Napi::Boolean::New(env, true);
}

Napi::Value Track::GetSubTracks(const Napi::CallbackInfo &info) {
	Napi::Array arr = Napi::Array::New(info.Env(), this->subTrackRefs.size());

	for (unsigned int i = 0; i < subTrackRefs.size(); i++) {
		arr[i] = subTrackRefs[i].Value();
	}

	return arr;
}

int Track::process(double *outputBuffer, double *inputBuffer, unsigned int nBufferFrames,
                   TimingInfo &ti) {
	// unsigned int sampleRate = dac->getStreamSampleRate();

	if (this->bufferSize < nBufferFrames) {
		this->outputBuffer = (double *)realloc(this->outputBuffer,
		                                       sizeof(double) * this->numChannels * nBufferFrames);
		this->bufferSize = nBufferFrames;
	}
	memset(this->outputBuffer, 0, sizeof(double) * this->numChannels * nBufferFrames);

	ScheduledTrackEvent *currentTrackEvent = this->trackEvents;

	// find the first event that should be playing
	while (currentTrackEvent != nullptr &&
	       ti.projectTimeSamples >
	           currentTrackEvent->time + currentTrackEvent->event->totalFrames) {
		currentTrackEvent = currentTrackEvent->next;
	}
	while (currentTrackEvent != nullptr &&
	       ti.projectTimeSamples + nBufferFrames >= currentTrackEvent->time &&
	       ti.projectTimeSamples <
	           currentTrackEvent->time + currentTrackEvent->event->totalFrames) {
		double *eventOutputBuffer = this->outputBuffer;
		double *eventInputBuffer = inputBuffer;
		unsigned int eventNBufferFrames = nBufferFrames;

		long nOffsetFrames = ti.projectTimeSamples - currentTrackEvent->time;
		if (nOffsetFrames < 0) {
			eventNBufferFrames += nOffsetFrames;
			eventOutputBuffer += (2 * -nOffsetFrames);
			eventInputBuffer += (2 * -nOffsetFrames);
			nOffsetFrames = 0;
		}

		currentTrackEvent->event->render(eventOutputBuffer, eventInputBuffer, eventNBufferFrames,
		                                 nOffsetFrames);

		currentTrackEvent = currentTrackEvent->next;
	}

	for (auto subTrack : subTracks) {
		subTrack->process(this->outputBuffer, inputBuffer, nBufferFrames, ti);
	}

	double levels[2] = {0, 0};
	for (unsigned int i = 0; i < nBufferFrames; i++) {
		const double sample1 = this->outputBuffer[i * 2] * this->volume;
		outputBuffer[i * 2] += sample1;
		if (fabs(sample1) > levels[0]) {
			levels[0] = fabs(sample1);
		}
		const double sample2 = this->outputBuffer[i * 2 + 1] * this->volume;
		outputBuffer[i * 2 + 1] += sample2;
		if (fabs(sample2) > levels[1]) {
			levels[1] = fabs(sample2);
		}
	}
	this->levels[0] = levels[0];
	this->levels[1] = levels[1];

	return 0;
}

ScheduledTrackEvent::ScheduledTrackEvent(unsigned long long time, AudioEvent *event,
                                         const Napi::Object &JSAudioEvent, bool triggerOnce)
    : time(time), event(event), triggerOnce(triggerOnce) {
	if (JSAudioEvent != nullptr) {
		audioEventJS = Napi::Reference<Napi::Object>::New(JSAudioEvent, 1);
	} else {
		audioEventJS = Napi::Reference<Napi::Object>();
	}
}

ScheduledTrackEvent::ScheduledTrackEvent(unsigned long long time, AudioEvent *event,
                                         bool triggerOnce)
    : time(time), event(event), triggerOnce(triggerOnce) {
	audioEventJS = Napi::Reference<Napi::Object>();
}

ScheduledTrackEvent::ScheduledTrackEvent(const ScheduledTrackEvent &e)
    : time(e.time), event(e.event), next(e.next), prev(e.prev), triggerOnce(e.triggerOnce) {
	if (e.audioEventJS.IsEmpty()) {
		audioEventJS = Napi::Reference<Napi::Object>();
	} else {
		audioEventJS = Napi::Reference<Napi::Object>::New(e.audioEventJS.Value(), 1);
	}
}

ScheduledTrackEvent::~ScheduledTrackEvent() {
	if (!audioEventJS.IsEmpty()) {
		audioEventJS.Unref();
	}
}
