{
  "targets": [
    {
      "target_name": "virplugs-audioengine",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "sources": [
		  "<!@(node -p \"require('glob').sync('./src/**/*.cc').join(' ')\")",
	   ],
      'xcode_settings': {
          'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
          'CLANG_CXX_LIBRARY': 'libc++',
          'MACOSX_DEPLOYMENT_TARGET': '10.7',
      },
      'msvs_settings': {
          'VCCLCompilerTool': { 'ExceptionHandling': 1 },
      },
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include.replace(/\\\\/g, '/')\")",
        "./deps/rtaudio",
        "./deps/rtaudio/include",
        "./deps/libsndfile/src",
        "./deps"
      ],
      #'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
      'dependencies': ["<!(node -p \"require('node-addon-api').gyp\")", "rtaudio", "libsndfile"],

      'conditions': [
          ['OS=="mac"', {
            'cflags+': ['-fvisibility=hidden'],
            'xcode_settings': {
            'GCC_SYMBOLS_PRIVATE_EXTERN': 'YES', # -fvisibility=hidden
            }
        }],
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
				'sources': [
					"deps/rtaudio/include/asio.cpp",
					"deps/rtaudio/include/asiodrivers.cpp",
					"deps/rtaudio/include/asiolist.cpp",
					"deps/rtaudio/include/iasiothiscallresolver.cpp",
				]
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
        ],
    },
	{
        "target_name": "libsndfile",
        'type': 'static_library',
        "cflags!": [ "-fno-exceptions" ],
        "cflags_cc!": [ "-fno-exceptions" ],

      "include_dirs": [
          './deps/libsndfile/src',
          './deps',
      ],

      'sources' : [ "<!@(node -p \"require('fs').readdirSync('./deps/libsndfile/src').filter(f=>!f.startsWith('test_')).map(f=>'./deps/libsndfile/src/'+f).join(' ')\
	  + ' ' + require('fs').readdirSync('./deps/libsndfile/src/ALAC').map(f=>'./deps/libsndfile/src/ALAC/'+f).join(' ')\
	  + ' ' + require('fs').readdirSync('./deps/libsndfile/src/G72x').map(f=>'./deps/libsndfile/src/G72x/'+f).join(' ')\
	  + ' ' + require('fs').readdirSync('./deps/libsndfile/src/GSM610').map(f=>'./deps/libsndfile/src/GSM610/'+f).join(' ')\")",
				],

    }
  ]
}
