# nice!duck status-screen simulator

Runs the real widget drawing code (`widgets/status.c`, `util.c`, `bolt.c` + the
`duck_idle` asset) on your Mac in an SDL window, so you can see how the screen
looks without flashing firmware. The widgets are compiled against small host
stubs (`stubs/zephyr/*`, `stubs/zmk/*`) that replace the Zephyr/ZMK headers.

## One-time setup

```sh
brew install sdl2 cmake
```

LVGL v9.2 is already vendored in `sim/lvgl/` (a shallow clone). If you want
pixel-identical output to the firmware, check out the same LVGL version ZMK
pulls into your west workspace:

```sh
cd sim/lvgl && git fetch --depth 1 origin <tag> && git checkout <tag>
```

## Build & run

```sh
cd sim
cmake -B build
cmake --build build
./build/nice_duck_sim
```

A window opens (6x zoom of the 160x68 panel) and cycles through mocked states
every 2.5s — the window title names the current one. Close the window or press
Ctrl+C in the terminal to quit.

## Adding / editing states

Edit the `presets[]` array in [main.c](main.c). Each entry is a
`struct status_state` (the same struct the firmware fills in), so you can mock
any combination of battery level, charging, BLE profile, layer, WPM, etc. The
field list lives in [widgets/util.h](../boards/shields/nice_duck_view/widgets/util.h).

## How it maps to the firmware

`main.c` calls the real `zmk_widget_status_init()` to build the three rotated
68x68 canvases exactly as the firmware does, then calls `draw_top/middle/bottom`
with each mocked state. The per-parameter init hooks
(`nice_duck_*_status_init`) are stubbed to no-ops because the sim supplies state
directly instead of subscribing to ZMK events.

If you add a new font, symbol, or LVGL feature to a widget, enable it in
[lv_conf.h](lv_conf.h) too.
