# wlhangul

A Hangul input method for Wayland.

## Building

Depends on libhangul, Wayland and libxkbcommon.

On Sway, this requires a [patch to add virtual-keyboard grabs
support][sway-keyboard-grab].

    meson build/
    ninja -C build/
    build/wlhangul

## License

MIT

[sway-keyboard-grab]: https://github.com/swaywm/sway/pull/4932
