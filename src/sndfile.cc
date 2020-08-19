#include <ctype.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sndfile.h>

#include <napi.h>

Napi::Value readAudioFileWaveform(const Napi::CallbackInfo &info) {
	SNDFILE *infile = NULL;
	SF_INFO sfinfo;

	memset(&sfinfo, 0, sizeof(sfinfo));

	const char *filename = info[0].As<Napi::String>().Utf8Value().c_str();
	const int window = info[1].As<Napi::Number>().Int32Value();

	if ((infile = sf_open(filename, SFM_READ, &sfinfo)) == NULL) {
		// printf("Not able to open input file %s.\n", infilename);
		// puts(sf_strerror(NULL));
		return info.Env().Null();
	};

	Napi::Array arr = Napi::Array::New(info.Env(), sfinfo.channels);

	sf_count_t snapshots = ceil((float)sfinfo.frames / window);
	for (int i = 0; i < sfinfo.channels; i++) {
		arr[i] = Napi::Array::New(info.Env(), snapshots);
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
			Napi::Object tuple = Napi::Object::New(info.Env());
			tuple.Set("max", max_val[i]);
			tuple.Set("min", min_val[i]);
			arr.Get(i).As<Napi::Array>().Set(current_snapshot, tuple);
		}
		current_snapshot++;
	};

	free(buf);
	free(max_val);
	free(min_val);
	sf_close(infile);

	return arr;
}

Napi::Value readAudioFileInfo(const Napi::CallbackInfo &info) {
	SNDFILE *infile = NULL;
	SF_INFO sfinfo;
	int full_precision = 0;

	memset(&sfinfo, 0, sizeof(sfinfo));

	const char *filename = info[0].As<Napi::String>().Utf8Value().c_str();

	if ((infile = sf_open(filename, SFM_READ, &sfinfo)) == NULL) {
		// printf("Not able to open input file %s.\n", infilename);
		// puts(sf_strerror(NULL));
		return info.Env().Null();
	};

	Napi::Object obj = Napi::Object::New(info.Env());

	obj.Set("filename", info[0]);
	obj.Set("channels", sfinfo.channels);
	obj.Set("samplerate", sfinfo.samplerate);
	obj.Set("frames", sfinfo.frames);
	obj.Set("sections", sfinfo.sections);
	obj.Set("seekable", sfinfo.seekable == 1);
	obj.Set("format", sfinfo.format);

	sf_close(infile);

	return obj;
}
