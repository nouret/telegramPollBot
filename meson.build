project('telegramPollBot', 'cpp'
	, version: '0.0.0'
	, default_options : ['cpp_std=c++17', 'werror=true', 'optimization=2']
	, meson_version: '>= 0.53'
	)

cxx = meson.get_compiler('cpp')
curl = cxx.find_library('curl')
rapidjson = dependency('RapidJSON')

executable('telegramPollBot', 'src/main.cpp'
	, dependencies: [curl]
	, cpp_args: ['-lcurl'])
