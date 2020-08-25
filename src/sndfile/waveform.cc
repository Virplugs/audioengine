#include <ctype.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <tuple>

#include <sndfile.h>

#include <napi.h>

class ReadAudioFileWaveformWorker : public Napi::AsyncWorker {
	typedef std::vector<std::vector<std::tuple<float, float>>> WaveformCache;

	public:
	ReadAudioFileWaveformWorker(Napi::Function &callback, const std::string &filename,
	                                         int window)
	    : Napi::AsyncWorker(callback), filename(filename), window(window) {
	}

	~ReadAudioFileWaveformWorker() {
	}

	// This code will be executed on the worker thread
	void Execute() override {
		memset(&sfinfo, 0, sizeof(sfinfo));
		SNDFILE *infile = NULL;

		if ((infile = sf_open(filename.c_str(), SFM_READ, &sfinfo)) == NULL) {
			SetError(sf_strerror(NULL));
			return;
		};

		cache = WaveformCache(sfinfo.channels);

		sf_count_t snapshots = ceil((float)sfinfo.frames / window);
		for (int i = 0; i < sfinfo.channels; i++) {
			cache[i] = std::vector<std::tuple<float, float>>(snapshots);
		}
		int current_snapshot = 0;
		float *buf = (float *)malloc(sizeof(float) * window * sfinfo.channels);
		float *max_val = (float *)malloc(sizeof(float) * sfinfo.channels);
		float *min_val = (float *)malloc(sizeof(float) * sfinfo.channels);
		int k, m, readcount;

		while ((readcount = sf_readf_float(infile, buf, window)) > 0) {
			memset(max_val, 0, sizeof(float) * sfinfo.channels);
			memset(min_val, 0, sizeof(float) * sfinfo.channels);
			for (k = 0; k < readcount; k++) {
				for (m = 0; m < sfinfo.channels; m++) {
					float val = buf[k * sfinfo.channels + m];
					if (val > max_val[m])
						max_val[m] = val;
					if (val < min_val[m])
						min_val[m] = val;
				}
			};
			for (int i = 0; i < sfinfo.channels; i++) {
				cache[i][current_snapshot] = std::make_tuple(max_val[i], min_val[i]);
			}
			current_snapshot++;
		};

		free(buf);
		free(max_val);
		free(min_val);

		sf_close(infile);
	}

	void OnOK() override {
		Napi::Array jsCache = Napi::Array::New(Env(), cache.size());

		for (unsigned int i = 0; i < cache.size(); i++) {
			Napi::Array jsChannel = Napi::Array::New(Env(), cache[i].size());

			for (unsigned int j = 0; j < cache[i].size(); j++) {
				Napi::Object tuple = Napi::Object::New(Env());
				tuple.Set("max", std::get<0>(cache[i][j]));
				tuple.Set("min", std::get<1>(cache[i][j]));

				jsChannel[j] = tuple;
			}

			jsCache[i] = jsChannel;
		}

		//Napi::HandleScope scope(Env());
		Callback().Call({Env().Null(), jsCache});
	}

  private:
	std::string filename;
	SF_INFO sfinfo;
	int window;
	WaveformCache cache;
};

Napi::Value readAudioFileWaveform(const Napi::CallbackInfo &info) {
	const Napi::Env env = info.Env();

	if (info.Length() < 3) {
		Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
		return env.Null();
	}

	if (!info[0].IsString() || !info[1].IsNumber() || !info[2].IsFunction()) {
		Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
		return env.Null();
	}

	const std::string filename = info[0].As<Napi::String>().Utf8Value();
	int window = info[1].As<Napi::Number>().Int32Value();
	Napi::Function cb = info[2].As<Napi::Function>();

	auto *wk = new ReadAudioFileWaveformWorker(cb, filename, window);
	wk->Queue();

	return info.Env().Undefined();
}
