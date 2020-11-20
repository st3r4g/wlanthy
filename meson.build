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

anthy = dependency('anthy')
anthyinput = cc.find_library('anthyinput')
wayland_client = dependency('wayland-client')
wayland_protocols = dependency('wayland-protocols')
xkbcommon = dependency('xkbcommon')

wlanthy_files = files('main.c', 'utf8.c')

subdir('protocol')

executable(
	meson.project_name(),
	wlanthy_files,
	include_directories: wlanthy_inc,
	dependencies: [anthy, anthyinput, wayland_client, xkbcommon],
	install: true,
)
