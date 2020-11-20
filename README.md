# wlanthy

Experimental, simple Wayland-native input method for japanese.
Born as a modification of [wlhangul].

## Building

Depends on anthy, Wayland and libxkbcommon.

On Sway, this requires a [patch to add virtual-keyboard grabs
support][sway-keyboard-grab].

    meson build/
    ninja -C build/
    ./build/wlanthy

Test with gtk3 apps (e.g. Firefox). By default it starts in passthrough mode.
Press F5 to toggle passthrough/japanese mode.

## License

MIT

[wlhangul]: https://github.com/emersion/wlhangul
[sway-keyboard-grab]: https://github.com/swaywm/sway/pull/4932
