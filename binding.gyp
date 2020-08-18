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
        "<!@(node -p \"require('node-addon-api').include\")",
        "./deps/rtaudio"
      ],
      #'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
      'dependencies': ["rtaudio"],
      
      'conditions': [
          ['OS=="mac"', {
            'cflags+': ['-fvisibility=hidden'],
            'xcode_settings': {
            'GCC_SYMBOLS_PRIVATE_EXTERN': 'YES', # -fvisibility=hidden
            }
        }]
      ],
    },
    {
        "target_name": "rtaudio",
        'type': 'static_library',
        "cflags!": [ "-fno-exceptions" ],
        "cflags_cc!": [ "-fno-exceptions" ],

        'conditions': [
          ['OS=="linux"', {
              'defines': [ '__UNIX_JACK__', '__LINUX_ALSA__' ]
          }],
          ['OS=="win"', {
              'defines': [ '__WINDOWS_ASIO__', '__WINDOWS_WASAPI__', '__WINDOWS_DS__' ],
              'msvs_settings': {
                    'VCCLCompilerTool': { 'ExceptionHandling': 1 },
                },
                "link_settings": {
                    "libraries": [
                        "-ldsound.lib",
                    ]
                },
          }],
          ['OS=="mac"', {
              'defines': [ '__MACOSX_CORE__' ],
              'xcode_settings': {
                    'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                    'CLANG_CXX_LIBRARY': 'libc++',
                    'MACOSX_DEPLOYMENT_TARGET': '10.7',
                },
          }],
      ],    

      "include_dirs": [
          './deps/rtaudio',
          './deps/rtaudio/include',
      ],

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