#pragma once

#include <napi.h>
#include <string>
#include <vector>

class Track;

#include "audioevent.hh"
#include "transport.hh"

struct ScheduledTrackEvent {
	ScheduledTrackEvent(unsigned long long time, AudioEvent *event,
	                    const Napi::Object &JSAudioEvent, bool triggerOnce = false);
	ScheduledTrackEvent(unsigned long long time, AudioEvent *event, bool triggerOnce = false);
	ScheduledTrackEvent(const ScheduledTrackEvent &);

	~ScheduledTrackEvent();

	unsigned long long time;
	AudioEvent *event;
	ScheduledTrackEvent *next = nullptr;
	ScheduledTrackEvent *prev = nullptr;
	bool triggerOnce;
	Napi::ObjectReference audioEventJS;
};

class Track : public Napi::ObjectWrap<Track> {
  public:
	static Napi::Object Init(Napi::Env env, Napi::Object exports);

	Track(const Napi::CallbackInfo &info);

	void playAudioEvent(AudioEvent *e);
	Napi::Value JS_playAudioEvent(const Napi::CallbackInfo &info);
	void stopAudioEvent(AudioEvent *e);
	Napi::Value JS_stopAudioEvent(const Napi::CallbackInfo &info);

	int process(double *outputBuffer, double *inputBuffer, unsigned int nBufferFrames);

	inline void setTransport(Transport *transport) {
		this->transport = transport;
	}

  protected:
	unsigned int numChannels;
	std::string name;
	ScheduledTrackEvent *trackEvents = nullptr;
	Transport *transport;

  private:
	std::vector<unsigned int> inputChannels, outputChannels;
	Napi::Value GetName(const Napi::CallbackInfo &info);
	void SetName(const Napi::CallbackInfo &info, const Napi::Value &value);

	Napi::Value GetInputChannels(const Napi::CallbackInfo &info);
	void SetInputChannels(const Napi::CallbackInfo &info, const Napi::Value &value);
	Napi::Value GetOutputChannels(const Napi::CallbackInfo &info);
	void SetOutputChannels(const Napi::CallbackInfo &info, const Napi::Value &value);

	void scheduleTrackEvent(const ScheduledTrackEvent &e);
	void cleanupScheduledTrackEvents();
};
