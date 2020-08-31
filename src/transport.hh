#pragma once

#include <napi.h>
#include <vector>

class Transport;

struct TimingInfo {
	unsigned long long projectTimeSamples;
	double projectTimeMusic;
	double projectTimeMusicBars;
	double barPositionMusic;
	double tempo;
	int timeSigNumerator;
	int timeSigDenominator;
	int samplerate;
};

#include "track.hh"

class Transport : public Napi::ObjectWrap<Transport> {
  public:
	static Napi::Object Init(Napi::Env env, Napi::Object exports);

	Transport(const Napi::CallbackInfo &info);

	virtual void Finalize(Napi::Env env) override;

	Track *masterTrack = nullptr;
	Napi::ObjectReference masterTrackRef;
	Track *cueTrack = nullptr;
	Napi::ObjectReference cueTrackRef;

	int loop(double *outputBuffer, double *inputBuffer, unsigned int nBufferFrames,
	         double streamTime);

	bool isPlaying = false;

	void start();
	void setTimeInSamples(unsigned long long);
	void stop();

  protected:
	friend class Track;
	TimingInfo timingInfo;
	unsigned long long runtimeSamples = 0;

  private:
	Napi::Value GetBPM(const Napi::CallbackInfo &info);
	void SetBPM(const Napi::CallbackInfo &info, const Napi::Value &value);

	Napi::Value GetMasterTrack(const Napi::CallbackInfo &info);
	void SetMasterTrack(const Napi::CallbackInfo &info, const Napi::Value &value);

	Napi::Value GetCueTrack(const Napi::CallbackInfo &info);
	void SetCueTrack(const Napi::CallbackInfo &info, const Napi::Value &value);

	Napi::Value GetIsPlaying(const Napi::CallbackInfo &info);
	void SetIsPlaying(const Napi::CallbackInfo &info, const Napi::Value &value);

	Napi::Value GetTimingInfo(const Napi::CallbackInfo &info);
	Napi::Value GetTotalRuntimeSamples(const Napi::CallbackInfo &info);

	Napi::Value JS_start(const Napi::CallbackInfo &info);
	Napi::Value JS_stop(const Napi::CallbackInfo &info);
	Napi::Value JS_setTimeInSamples(const Napi::CallbackInfo &info);

	inline void resetTimingInfo() {
		this->timingInfo.barPositionMusic = 0;
		this->timingInfo.projectTimeMusic = 0;
		this->timingInfo.projectTimeMusicBars = 0;
		this->timingInfo.projectTimeSamples = 0;
	}
};

extern Transport *activeTransport;
