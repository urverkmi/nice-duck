# nice-duck

A ZMK config repo for a **Lily58** with a **custom nice!view status screen** on
the left (central) half, running on **nice!nano v2** controllers.

It's both a ZMK config (`config/`, `build.yaml`) and a small ZMK module: it ships
a custom display shield, **`nice_duck_view`**, that replaces the stock `nice_view`
widget so we can draw our own screen. (Defining your own status screen alongside
the stock one would clash; shipping a replacement shield is the clean way around
it — same approach as nice-view-gem / zmk-nice-oled.)

The screen is a **skeleton**: it already extracts every parameter and renders a
basic placeholder for each. Design the visuals on top of it — see
[docs/CUSTOMIZING.md](docs/CUSTOMIZING.md).

## What it shows (left/central screen)

| Zone (canvas) | Shows |
|---------------|-------|
| top    | central battery + charging, right-half battery, BLE/USB output symbol |
| middle | Bluetooth profile circles (1–5, selected/connected/bonded), right-half link dot |
| bottom | active layer, Caps Lock, WPM |

The nice!view is a 160×68 Sharp memory LCD, 1-bit (black/white), mounted rotated —
so the design space is effectively **68 wide × 160 tall**. Each zone is a 68×68
canvas drawn naturally and rotated 270° before display.

## Layout

```
config/
  west.yml          # pulls in zmkfirmware/zmk
  lily58.conf       # feature flags (display, HID indicators, split battery, WPM)
build.yaml          # CI build matrix (left = custom screen, right = stock)
zephyr/module.yml   # registers this repo as a module so the shield is found
boards/shields/nice_duck_view/
  Kconfig.shield / Kconfig.defconfig
  nice_duck_view.overlay     # the LS011B7DH03 display node
  CMakeLists.txt
  custom_status_screen.c     # zmk_display_status_screen() entry point
  widgets/
    status.{c,h}             # the widget: 3 canvases + draw_top/middle/bottom
    util.{c,h}               # LVGL helpers + canvas rotation + shared state
    bolt.c                   # charging icon (example converted bitmap)
    battery_status.c         # central battery        (event)
    output_status.c          # BLE/USB + profiles     (event)
    peripheral_status.c      # right-half link        (event)
    hid_status.c             # Caps Lock              (event)
    layer_status.c           # active layer           (event)
    wpm_status.c             # typing speed (WPM)     (event)
    poll_status.c            # peripheral battery     (polled)
  assets/                    # your converted graphics (+ how-to)
docs/CUSTOMIZING.md          # step-by-step: graphics + binding to state
```

## How a parameter flows to the screen

Every value follows the ZMK widget pattern: a small **state struct** → a
**get_state()** that reads ZMK → an **update callback** that writes into the
shared `struct status_state` and calls the right `draw_*`. Event-driven widgets
use the `ZMK_DISPLAY_WIDGET_LISTENER` + `ZMK_SUBSCRIPTION` macros; the peripheral
battery has no ZMK event, so it is refreshed on a timer in `poll_status.c`.

### The extraction APIs (cheat sheet)

| Parameter            | API call                                                | Event subscribed |
|----------------------|---------------------------------------------------------|------------------|
| Central battery      | `zmk_battery_state_of_charge()`, `zmk_usb_is_powered()` | `zmk_battery_state_changed`, `zmk_usb_conn_state_changed` |
| Peripheral battery   | `zmk_split_central_get_peripheral_battery_level(0, &lvl)` | *(polled)* |
| Output transport     | `zmk_endpoint_get_selected()`                           | `zmk_endpoint_changed` |
| Active BLE profile   | `zmk_ble_active_profile_index/_is_connected/_is_open()` | `zmk_ble_active_profile_changed` |
| Per-profile state    | `zmk_ble_profile_is_connected/_is_open(i)`              | `zmk_ble_active_profile_changed` |
| Right-half link      | event `.connected`                                      | `zmk_split_peripheral_status_changed` |
| Active layer         | `zmk_keymap_highest_layer_active()`, `zmk_keymap_layer_name()` | `zmk_layer_state_changed` |
| Caps Lock            | `zmk_hid_indicators_get_current_profile() & BIT(1)`     | `zmk_hid_indicators_changed` |
| WPM                  | `zmk_wpm_get_state()`                                   | `zmk_wpm_state_changed` |

## Build

This repo is built by the **ZMK GitHub build action** (the matrix in
`build.yaml`); just push and download the `.uf2` artifacts.

> The included `.github/workflows/build.yml` tracks **`@main`**, and
> `config/west.yml` pins zmk to `main`. They must stay in sync: the widget code
> uses LVGL 9 / current ZMK APIs, so the older `@v0.3` workflow fails with
> "Invalid BOARD; nice_nano_v2" (it can't parse main's newer board format).

To build a half locally with a ZMK west workspace:

```
west build -b nice_nano -- -DSHIELD="lily58_left nice_view_adapter nice_duck_view"
```

A clean link confirms there's no duplicate status-screen symbol and that all the
ZMK APIs resolve. Flash `build/zephyr/zmk.uf2` to the left half.

## Notes

- **Caps Lock** needs `CONFIG_ZMK_HID_INDICATORS=y` (already set).
- **WPM** needs `CONFIG_ZMK_WPM=y` (already set).
- **Peripheral battery** needs `CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING=y`
  (already set) and battery reporting on the right half.
- Code in `widgets/util.c`, `widgets/bolt.c`, and the canvas/rotation approach are
  adapted from the stock ZMK nice_view widget (MIT, The ZMK Contributors).
