project(
	'wlanthy',
	'c',
	version: '0.0.0',
	license: 'MIT',
	meson_version: '>=0.51.0',
	default_options: [
		'c_std=c99',
		'warning_level=2',
		'werror=false',
	],
)

cc = meson.get_compiler('c')

add_project_arguments(cc.get_supported_arguments([
	'-Wno-missing-braces',
	'-Wno-unused-parameter',
]), language: 'c')

wlanthy_inc = include_directories('include')

wlanthy_files = files('main.c', 'utf8.c', 'log.c')

deps = []
anthy = dependency('anthy')
deps += anthy
if get_option('input') == 'anthy'
	deps += cc.find_library('anthyinput')
else
	add_project_arguments('-DHAVE_UTF8=1', language: 'c')
	wlanthy_files += files('input.c')
endif
deps += dependency('wayland-client')
wayland_protocols = dependency('wayland-protocols')
deps += wayland_protocols
deps += dependency('xkbcommon')

subdir('protocol')

executable(
	meson.project_name(),
	wlanthy_files,
	include_directories: wlanthy_inc,
	dependencies: deps,
	install: true,
)
