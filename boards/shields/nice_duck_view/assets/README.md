# Graphics → bitmap assets

This folder holds the images your screen draws. ZMK uses **LVGL 9**, and the
nice!view is a **1-bit (black/white) memory LCD** — no greys. Source art here,
generated C arrays here too.

## 1. Draw it

- Work in pure **black & white**, no anti-aliasing/greys.
- Target the pixel size of the spot it goes in. The whole usable screen is
  **68 wide × 160 tall** (the panel is mounted rotated). A per-widget icon is
  usually 16–40 px.
- Keep strokes ≥ 2 px so they survive on the low-res panel.

## 2. (Optional) pre-rotate

The widgets rotate each canvas 270° at runtime (`rotate_canvas()` in
`widgets/util.c`), so you can draw in the natural reading orientation and let the
firmware handle it. If you'd rather save a little CPU/RAM, rotate the source art
−90° here and skip the runtime rotation — pick one, don't do both.

## 3. Convert to a C array

Use the LVGL image converter: <https://lvgl.io/tools/imageconverter>

- **LVGL version:** 9
- **Color format:** `I1` (1-bit indexed) for solid icons, or `A1` if you want a
  transparent mask that only paints "on" pixels.
- **Output:** C array.

Or the CLI from the lvgl repo:

```
python LVGLImage.py my_icon.png --cf I1 --ofmt C -o .
```

`example_icon.c` / `example_icon.h` in this folder show exactly what the output
looks like, and `widgets/bolt.c` is a second real example (I2 format).

## 4. Wire it in

1. Drop the generated `my_icon.c` into this folder.
2. Add a declaration to a header (or reuse `example_icon.h`):
   `extern const lv_img_dsc_t my_icon;`
3. Add it to the build in `../CMakeLists.txt`:
   `zephyr_library_sources(assets/my_icon.c)`
4. Draw it from a `draw_*` routine in `../widgets/status.c`:
   `canvas_draw_img(canvas, x, y, &my_icon, &img_dsc);`

See `../../../../docs/CUSTOMIZING.md` for the full edit-by-parameter workflow.
