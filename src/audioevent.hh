#pragma once

#include <vector>
#include <string>
#include <napi.h>
#include <sndfile.h>

class AudioEvent : public Napi::ObjectWrap<AudioEvent> {
	public:
	  static Napi::Object Init(Napi::Env env, Napi::Object exports);

	  AudioEvent(const Napi::CallbackInfo &info);

	  virtual void Finalize(Napi::Env env) override;

	  int render(double *outputBuffer, double *inputBuffer, unsigned int nBufferFrames, unsigned int nOffsetFrames);

	  double duration = 0.0;
	  unsigned long totalFrames = 1;
	  unsigned long lastFrameOffset = 0;
	  double gain = 1.0;

	protected:
	  std::string name, filename;

	private:
	  Napi::Value GetName(const Napi::CallbackInfo &info);
	  void SetName(const Napi::CallbackInfo &info, const Napi::Value &value);
	  Napi::Value GetLastFrameOffset(const Napi::CallbackInfo &info);
	  Napi::Value GetDuration(const Napi::CallbackInfo &info);
	  Napi::Value GetTotalFrames(const Napi::CallbackInfo &info);
	  Napi::Value GetGain(const Napi::CallbackInfo &info);
	  void SetGain(const Napi::CallbackInfo &info, const Napi::Value &value);

	  SF_INFO sfinfo;
	  SNDFILE *infile = NULL;
      };
