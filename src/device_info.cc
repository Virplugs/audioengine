#include <iostream>
#include <vector>

#include <RtAudio.h>
#include <napi.h>

Napi::String Method(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	return Napi::String::New(env, "world!");
}

Napi::Object getDeviceInfo(const Napi::CallbackInfo &info) {
	std::vector<RtAudio::Api> apis;
	RtAudio::getCompiledApi(apis);

	Napi::Object obj = Napi::Object::New(info.Env());

	for (const RtAudio::Api &api : apis) {
		Napi::Object apiobj = Napi::Object::New(info.Env());
		apiobj.Set("displayName", RtAudio::getApiDisplayName(api));
		RtAudio audio(api);
		// Determine the number of devices available
		unsigned int devices = audio.getDeviceCount();
		apiobj.Set("numDevices", devices);

		Napi::Array devarr = Napi::Array::New(info.Env());
		// Scan through devices for various capabilities
		for (unsigned int i = 0; i < devices; i++) {
			RtAudio::DeviceInfo dinfo = audio.getDeviceInfo(i);
			if (dinfo.probed == true) {
				Napi::Object devobj = Napi::Object::New(info.Env());
				devobj.Set("name", dinfo.name);
				devobj.Set("outputChannels", dinfo.outputChannels);
				devobj.Set("inputChannels", dinfo.inputChannels);
				devobj.Set("duplexChannels", dinfo.duplexChannels);
				devobj.Set("isDefaultInput", dinfo.isDefaultInput);
				devobj.Set("isDefaultOutput", dinfo.isDefaultOutput);
				devobj.Set("preferredSampleRate", dinfo.preferredSampleRate);

				Napi::Array sampleRates = Napi::Array::New(info.Env(), dinfo.sampleRates.size());
				for (unsigned int i = 0; i < dinfo.sampleRates.size(); i++) {
					sampleRates[i] = dinfo.sampleRates[i];
				}
				devobj.Set("sampleRates", sampleRates);

				Napi::Array nativeFormats = Napi::Array::New(info.Env());
				if (dinfo.nativeFormats & RTAUDIO_SINT8)
					nativeFormats[nativeFormats.Length()] = "SINT8";
				if (dinfo.nativeFormats & RTAUDIO_SINT16)
					nativeFormats[nativeFormats.Length()] = "SINT16";
				if (dinfo.nativeFormats & RTAUDIO_SINT24)
					nativeFormats[nativeFormats.Length()] = "SINT24";
				if (dinfo.nativeFormats & RTAUDIO_SINT32)
					nativeFormats[nativeFormats.Length()] = "SINT32";
				if (dinfo.nativeFormats & RTAUDIO_FLOAT32)
					nativeFormats[nativeFormats.Length()] = "FLOAT32";
				if (dinfo.nativeFormats & RTAUDIO_FLOAT64)
					nativeFormats[nativeFormats.Length()] = "FLOAT64";
				devobj.Set("nativeFormats", nativeFormats);

				devarr[devarr.Length()] = devobj;
			}
		}
		apiobj.Set("devices", devarr);
		obj.Set(RtAudio::getApiName(api), apiobj);
	}

	return obj;
}
