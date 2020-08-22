#include <ctype.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>

#include <sndfile.h>

#include <napi.h>

class ReadAudioFileInfoWorker : public Napi::AsyncWorker {
  public:
	ReadAudioFileInfoWorker(Napi::Function &callback, const std::string &filename)
	    : Napi::AsyncWorker(callback), filename(filename) {
	}

	~ReadAudioFileInfoWorker() {
	}

	// This code will be executed on the worker thread
	void Execute() override {
		memset(&sfinfo, 0, sizeof(sfinfo));
		SNDFILE *infile = NULL;
		if ((infile = sf_open(filename.c_str(), SFM_READ, &sfinfo)) == NULL) {
			SetError(sf_strerror(NULL));
			return;
		};
		sf_close(infile);
	}

	void OnOK() override {
		Napi::Object obj = Napi::Object::New(Env());

		obj.Set("filename", filename);
		obj.Set("channels", sfinfo.channels);
		obj.Set("samplerate", sfinfo.samplerate);
		obj.Set("frames", sfinfo.frames);
		obj.Set("sections", sfinfo.sections);
		obj.Set("seekable", sfinfo.seekable == 1);
		obj.Set("format", sfinfo.format);

		//Napi::HandleScope scope(Env());
		try {
		Callback().Call({Env().Null(), obj});
		} catch(Napi::Error &e) {
			printf("bladibla\n");
		}
	}

  private:
	std::string filename;
	SF_INFO sfinfo;
};

Napi::Value readAudioFileInfo(const Napi::CallbackInfo &info) {
	if (info.Length() < 2) {
		Napi::TypeError::New(info.Env(), "Wrong number of arguments").ThrowAsJavaScriptException();
		return info.Env().Null();
	}

	if (!info[0].IsString() || !info[1].IsFunction()) {
		Napi::TypeError::New(info.Env(), "Wrong arguments").ThrowAsJavaScriptException();
		return info.Env().Null();
	}

	const std::string filename = info[0].As<Napi::String>().Utf8Value();
	Napi::Function cb = info[1].As<Napi::Function>();
	ReadAudioFileInfoWorker *wk = new ReadAudioFileInfoWorker(cb, filename);
	wk->Queue();
	return info.Env().Undefined();
}
