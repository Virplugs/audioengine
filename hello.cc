#include <iostream>
#include <vector>

#include <napi.h>
#include <RtAudio.h>

Napi::String Method(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    return Napi::String::New(env, "world!");
}

Napi::Value printDeviceInfo(const Napi::CallbackInfo &info)
{
    std::vector<RtAudio::Api> apis;
    RtAudio::getCompiledApi(apis);

    for (const RtAudio::Api &api : apis) {
        std::cout << "API: " << RtAudio::getApiDisplayName(api) << "\n";
        RtAudio audio(api);
        // Determine the number of devices available
        unsigned int devices = audio.getDeviceCount();
        // Scan through devices for various capabilities
        for (unsigned int i = 0; i < devices; i++)
        {
            RtAudio::DeviceInfo dinfo = audio.getDeviceInfo(i);
            if (dinfo.probed == true)
            {
                // Print, for example, the maximum number of output channels for each device
                std::cout << "device = " << i;
                std::cout << " (" << dinfo.name << ") ";
                std::cout << ": maximum output channels = " << dinfo.outputChannels << "\n";
            }
        }
        std::cout << "\n";
    }

    return info.Env().Undefined();
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports.Set(Napi::String::New(env, "hello"), Napi::Function::New(env, Method));
    exports.Set(Napi::String::New(env, "printDeviceInfo"), Napi::Function::New(env, printDeviceInfo));
    return exports;
}

NODE_API_MODULE(hello, Init)
