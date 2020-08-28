#pragma once

#include <napi.h>
#include <vector>

class Transport;

#include "track.hh"

class Transport : public Napi::ObjectWrap<Transport> {
  public:
	static Napi::Object Init(Napi::Env env, Napi::Object exports);

	Transport(const Napi::CallbackInfo &info);

	virtual void Finalize(Napi::Env env) override;

	unsigned int bpm = 120;
	unsigned long long frameCount = 0;
	Track *masterTrack = nullptr;
	Napi::ObjectReference masterTrackRef;
	Track *cueTrack = nullptr;
	Napi::ObjectReference cueTrackRef;

	int loop(double *outputBuffer, double *inputBuffer, unsigned int nBufferFrames,
	         double streamTime);

  protected:

  private:
	Napi::Value GetBPM(const Napi::CallbackInfo &info);
	void SetBPM(const Napi::CallbackInfo &info, const Napi::Value &value);

	Napi::Value GetMasterTrack(const Napi::CallbackInfo &info);
	void SetMasterTrack(const Napi::CallbackInfo &info, const Napi::Value &value);

	Napi::Value GetCueTrack(const Napi::CallbackInfo &info);
	void SetCueTrack(const Napi::CallbackInfo &info, const Napi::Value &value);
};

extern Transport *activeTransport;
