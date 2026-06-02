# Customizing your nice-duck screen

A task-oriented guide. For each parameter there are **two** places to touch:

- **Extraction** — `boards/shields/nice_duck_view/widgets/<x>_status.c`
  (reads the value from ZMK, stores it in `struct status_state`).
- **Drawing** — `boards/shields/nice_duck_view/widgets/status.c`, in one of
  `draw_top` / `draw_middle` / `draw_bottom` (turns the value into pixels).

## Where each parameter lives

| Parameter            | Extraction file          | Updates on        | Drawn in    |
|----------------------|--------------------------|-------------------|-------------|
| Central battery      | `battery_status.c`       | event             | `draw_top`    |
| Peripheral battery   | `light_status.c` (poll)  | timer (2 s)       | `draw_top`    |
| BLE/USB output       | `output_status.c`        | event             | `draw_top`    |
| BT profile circles   | `output_status.c`        | event             | `draw_middle` |
| Right-half link      | `peripheral_status.c`    | event             | `draw_middle` |
| Active layer         | `layer_status.c`         | event             | `draw_bottom` |
| Caps Lock            | `hid_status.c`           | event             | `draw_bottom` |
| Light (RGB/backlight)| `light_status.c` (poll)  | timer (2 s)       | `draw_bottom` |

> The three canvases are 68×68 squares rotated 270° at draw time. Everything in a
> `draw_*` function is in natural (un-rotated) coordinates, origin top-left.

---

## A. Convert a graphic to a bitmap

- [ ] Draw it **1-bit black/white** at the target size (whole screen is 68×160).
- [ ] (Optional) rotate the source −90° to skip runtime rotation.
- [ ] Convert at <https://lvgl.io/tools/imageconverter> → **LVGL 9**, format
      **I1** (or **A1** for a transparent mask), output **C array**.
      (CLI: `python LVGLImage.py icon.png --cf I1 --ofmt C -o assets/`)
- [ ] Save the `.c` into `boards/shields/nice_duck_view/assets/`.
- [ ] Declare it: add `extern const lv_img_dsc_t icon;` to `assets/example_icon.h`
      (or a new header).
- [ ] Compile it: add `zephyr_library_sources(assets/icon.c)` to
      `boards/shields/nice_duck_view/CMakeLists.txt`.

See `boards/shields/nice_duck_view/assets/README.md` and the worked examples
`assets/example_icon.c` (I1) and `widgets/bolt.c` (I2).

## B. Show a graphic based on a parameter

The value is already in `state` inside the `draw_*` function. Pick how to render:

- [ ] **On/off icon swap** (best for your designed art) — in `draw_bottom`,
      replace the `"CAPS"` text with an image:
      ```c
      #include "../assets/example_icon.h"   // top of status.c
      // inside draw_bottom:
      if (state->caps_lock) {
          lv_draw_image_dsc_t img_dsc;
          lv_draw_image_dsc_init(&img_dsc);
          canvas_draw_img(canvas, 30, 24, &example_icon, &img_dsc);
      }
      ```
- [ ] **Pick from a set by index** — e.g. a battery sprite per 20%:
      ```c
      static const lv_img_dsc_t *batt_frames[] = {
          &batt_0, &batt_20, &batt_40, &batt_60, &batt_80, &batt_100,
      };
      canvas_draw_img(canvas, 0, 2, batt_frames[state->battery / 20], &img_dsc);
      ```
- [ ] **Dynamic shape** (no asset) — already used for the battery fill and the
      profile arcs; drive a size/length from the value with `canvas_draw_rect` /
      `canvas_draw_arc`.
- [ ] Rebuild and reflash the left half to see it (see README "Build").

## C. Add a brand-new parameter

- [ ] Add a field to `struct status_state` in `widgets/util.h`.
- [ ] Copy an existing extraction file as a template:
      - has a ZMK event? copy `hid_status.c` (uses `ZMK_DISPLAY_WIDGET_LISTENER`
        + `ZMK_SUBSCRIPTION`).
      - no event? add it to the poll loop in `light_status.c`.
- [ ] Point its `set_*` at the right `draw_*` canvas.
- [ ] Declare its `nice_duck_<x>_status_init` in `widgets/status.h` and call it
      from `zmk_widget_status_init` in `status.c`.
- [ ] Add the new `.c` to `CMakeLists.txt`.
- [ ] Draw the new field inside the relevant `draw_*`.

The ZMK APIs/events for common parameters are listed in the project `README.md`.

## D. Move things around / change the layout

- [ ] **Within a canvas:** change the `x, y` args in the `canvas_draw_*` calls in
      the relevant `draw_*` function.
- [ ] **Whole zones:** change the `lv_obj_align(...)` offsets for `top` / `middle`
      / `bottom` in `zmk_widget_status_init` (`status.c`).
- [ ] **Invert colors** (white-on-black): build with
      `CONFIG_NICE_DUCK_WIDGET_INVERTED=y` in `config/lily58.conf`.

## E. Turn a feature off

- [ ] Comment out its `nice_duck_<x>_status_init();` call in
      `zmk_widget_status_init` (`status.c`), and stop drawing it in the `draw_*`.
