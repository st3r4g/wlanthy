project(
	'wlhangul',
	'c',
	version: '0.0.0',
	license: 'MIT',
	meson_version: '>=0.51.0',
	default_options: [
		'c_std=c99',
		'warning_level=2',
		'werror=true',
	],
)

cc = meson.get_compiler('c')

add_project_arguments(cc.get_supported_arguments([
	'-Wno-missing-braces',
	'-Wno-unused-parameter',
]), language: 'c')

wlhangul_inc = include_directories('include')

libhangul = dependency('libhangul')
wayland_client = dependency('wayland-client')
wayland_protocols = dependency('wayland-protocols')
xkbcommon = dependency('xkbcommon')

wlhangul_files = files('main.c', 'utf8.c')

subdir('protocol')

executable(
	meson.project_name(),
	wlhangul_files,
	include_directories: wlhangul_inc,
	dependencies: [libhangul, wayland_client, xkbcommon],
	install: true,
)
