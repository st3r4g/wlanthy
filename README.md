# wlanthy

Experimental, simple Wayland-native Japanese input method. Born as a
modification of [wlhangul].

## Building

Depends on `anthy`, `wayland` and `libxkbcommon`.

On Sway, this requires a [patch to add virtual-keyboard grabs
support][sway-keyboard-grab].

    meson build/
    ninja -C build/
    ./build/wlanthy

By default it starts in anthy mode, so you should be able to start typing
in japanese right away after giving focus to a supported application.

Supported applications are those that implement the `text-input` protocol,
like gtk+3 applications (e.g. Firefox).

Press F12 to toggle anthy/passthrough mode. Passthrough mode is not meant
for regular usage (it's slow), I think it would be better to enable/disable
the input method from the compositor (but afaik Sway doesn't do this atm).

## License

MIT

[wlhangul]: https://github.com/emersion/wlhangul
[sway-keyboard-grab]: https://github.com/swaywm/sway/pull/4932
