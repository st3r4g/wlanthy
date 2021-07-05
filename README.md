# wlanthy

Experimental, simple Wayland-native Japanese input method, using protocols
implemented by `wlroots`. Born as a modification of [wlhangul].

On Sway, this requires version >= **1.6**.

## Building

Depends on `anthy`, `wayland` and `libxkbcommon`.

    meson build/
    ninja -C build/
    ./build/wlanthy

By default it starts in anthy mode, so you should be able to start typing
in japanese right away after giving focus to a supported application.

Supported applications are those that implement the `text-input-v3` protocol:
* `gtk+3` applications (e.g. Firefox, GNOME apps)
* [foot] terminal emulator, since 1.6.0

Unfortunately, at the moment `qt5` implements `text-input-v2` which is not
implemented by `wlroots`.

## Usage

Romaji input is immediately turned into kana. Press `Space` to subdivide the
sentence into segments called phrases (文節). Use `Tab` or `Left/Right`
to navigate between phrases: for a given phrase you can press `Space` or
`Up/Down` to change candidates and use `Alt+Tab` to resize it. Undo with
`Backspace`. Commit with `Enter` (or by starting to type the next sentence).

By default romaji input is mapped into Hiragana. Other mappings can be
selected with the following keys:

| `F5`     | `F6`     | `F7`       | `F8`       |
|----------|----------|------------|------------|
| Hiragana | Katakana | Half-Width | Full-Width |

Press F12 to toggle anthy/passthrough mode. Passthrough mode is slow because
the key has to travel back-and-forth between wlanthy and the compositor, so it
is recommended to instead bind a keyboard shortcut to a script that toggles the
wlanthy service.

## License

MIT

[wlhangul]: https://github.com/emersion/wlhangul
[foot]: https://codeberg.org/dnkl/foot
