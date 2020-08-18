{
  "targets": [
    {
      "target_name": "virplugs-audioengine",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "sources": [ "hello.cc" ],
      'xcode_settings': {
          'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
          'CLANG_CXX_LIBRARY': 'libc++',
          'MACOSX_DEPLOYMENT_TARGET': '10.7',
      },
      'msvs_settings': {
          'VCCLCompilerTool': { 'ExceptionHandling': 1 },
      },
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
      'dependencies': ["rtaudio"]
    },
    {
        "target_name": "rtaudio",
        'type': 'static_library',
        "cflags!": [ "-fno-exceptions" ],
        "cflags_cc!": [ "-fno-exceptions" ],
          'xcode_settings': {
                'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                'CLANG_CXX_LIBRARY': 'libc++',
                'MACOSX_DEPLOYMENT_TARGET': '10.7',
            },
            'msvs_settings': {
                'VCCLCompilerTool': { 'ExceptionHandling': 1 },
            },

        "sources": [ 
            "deps/rtaudio/RtAudio.cpp",
            "deps/rtaudio/rtaudio_c.cpp", 
            "deps/rtaudio/include/asio.cpp", 
            "deps/rtaudio/include/asiodrivers.cpp", 
            "deps/rtaudio/include/asiolist.cpp", 
            "deps/rtaudio/include/iasiothiscallresolver.cpp", 
        ],
    }
  ]
}