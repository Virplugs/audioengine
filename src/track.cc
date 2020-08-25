#include "audioengine.hh"

#include "track.hh"

Napi::Object Track::Init(Napi::Env env, Napi::Object exports) {
	exports.Set("Track",
	            DefineClass(env, "Track",
	                        {
	                            InstanceMethod<&Track::JS_playAudioEvent>("playAudioEvent"),
	                            InstanceMethod<&Track::JS_stopAudioEvent>("stopAudioEvent"),
	                            // InstanceMethod<&Track::SetValue>("SetValue"),
	                            InstanceAccessor("name", &Track::GetName, &Track::SetName),
	                            InstanceAccessor("inputChannels", &Track::GetInputChannels,
	                                             &Track::SetInputChannels),
	                            InstanceAccessor("outputChannels", &Track::GetOutputChannels,
	                                             &Track::SetOutputChannels),
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

Napi::Value Track::GetName(const Napi::CallbackInfo &info) {
	return Napi::String::New(info.Env(), this->name);
}

void Track::SetName(const Napi::CallbackInfo &info, const Napi::Value &value) {
	this->name = value.As<Napi::String>().Utf8Value();
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
	auto s = ScheduledTrackEvent(transport->frameCount + 256,
	                             AudioEvent::Unwrap(info[0].As<Napi::Object>()), true,
	                             &info[0].As<Napi::Object>());
	this->scheduleTrackEvent(s);
	return info.Env().Undefined();
}

void Track::playAudioEvent(AudioEvent *e) {
	this->scheduleTrackEvent(ScheduledTrackEvent(transport->frameCount + 256, e, true));
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
		if (transport->frameCount >
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

int Track::process(double *outputBuffer, double *inputBuffer, unsigned int nBufferFrames) {
	unsigned int sampleRate = dac->getStreamSampleRate();

	ScheduledTrackEvent *currentTrackEvent = this->trackEvents;

	// find the first event that should be playing
	while (currentTrackEvent != nullptr &&
	       transport->frameCount >
	           currentTrackEvent->time + currentTrackEvent->event->totalFrames) {
		currentTrackEvent = currentTrackEvent->next;
	}
	while (currentTrackEvent != nullptr &&
	       transport->frameCount <
	           currentTrackEvent->time + currentTrackEvent->event->totalFrames) {
		double *eventOutputBuffer = outputBuffer;
		double *eventInputBuffer = inputBuffer;
		unsigned int eventNBufferFrames = nBufferFrames;

		long nOffsetFrames = transport->frameCount - currentTrackEvent->time;
		if (nOffsetFrames < 0) {
			eventNBufferFrames += nOffsetFrames;
			eventOutputBuffer += (2 * -nOffsetFrames);
			eventInputBuffer += (2 * -nOffsetFrames);
		}

		currentTrackEvent->event->render(eventOutputBuffer, eventInputBuffer, eventNBufferFrames,
		                                 nOffsetFrames);

		currentTrackEvent = currentTrackEvent->next;
	}

	return 0;
}

ScheduledTrackEvent::ScheduledTrackEvent(unsigned long long time, AudioEvent *event,
                                         bool triggerOnce, Napi::Object *JSAudioEvent)
    : time(time), event(event), triggerOnce(triggerOnce) {
	if (JSAudioEvent != nullptr) {
		audioEventJS = Napi::Reference<Napi::Object>::New(*JSAudioEvent, 1);
	} else {
		audioEventJS = Napi::Reference<Napi::Object>::Reference();
	}
}

ScheduledTrackEvent::ScheduledTrackEvent(const ScheduledTrackEvent &e)
    : time(e.time), event(e.event), triggerOnce(e.triggerOnce), prev(e.prev), next(e.next) {
	if (e.audioEventJS.IsEmpty()) {
		audioEventJS = Napi::Reference<Napi::Object>::Reference();
	} else {
		audioEventJS = Napi::Reference<Napi::Object>::New(e.audioEventJS.Value(), 1);
	}
}

ScheduledTrackEvent::~ScheduledTrackEvent() {
	if (!audioEventJS.IsEmpty()) {
		audioEventJS.Unref();
	}
}
