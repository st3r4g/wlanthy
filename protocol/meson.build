protocols_dir = wayland_protocols.get_variable(pkgconfig: 'pkgdatadir')

wayland_scanner = dependency('wayland-scanner', native: true)
wayland_scanner_exe = find_program(
  wayland_scanner.get_variable(pkgconfig: 'wayland_scanner'),
  native: true,
)

protocols = {
  'input-method-unstable-v2': 'input-method-unstable-v2.xml',
  'virtual-keyboard-unstable-v1': 'virtual-keyboard-unstable-v1.xml',
}

foreach name, path : protocols
	code = custom_target(
		name.underscorify() + '_c',
		input: path,
		output: '@BASENAME@-protocol.c',
		command: [wayland_scanner_exe, 'private-code', '@INPUT@', '@OUTPUT@'],
	)
	wlanthy_files += code

	client_header = custom_target(
		name.underscorify() + '_client_h',
		input: path,
		output: '@BASENAME@-client-protocol.h',
		command: [wayland_scanner_exe, 'client-header', '@INPUT@', '@OUTPUT@'],
	)
	wlanthy_files += client_header
endforeach
