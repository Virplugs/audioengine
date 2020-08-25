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
	std::vector<Track *> tracks;

	int loop(double *outputBuffer, double *inputBuffer, unsigned int nBufferFrames,
	         double streamTime);

  protected:
	Napi::Reference<Napi::Array> tracksRef;

  private:
	Napi::Value GetBPM(const Napi::CallbackInfo &info);
	void SetBPM(const Napi::CallbackInfo &info, const Napi::Value &value);

	Napi::Value GetTracks(const Napi::CallbackInfo &info);
	void SetTracks(const Napi::CallbackInfo &info, const Napi::Value &value);
};

extern Transport *activeTransport;
