/*
 * LVGL drawing helpers + the shared status_state. Adapted from the stock zmk
 * nice_view widget (MIT, The ZMK Contributors) for LVGL 9.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <lvgl.h>
#include <zmk/endpoints.h>

#define NICEVIEW_PROFILE_COUNT 5

#define CANVAS_SIZE 68
#define CANVAS_COLOR_FORMAT LV_COLOR_FORMAT_L8 // smallest type supported by sw_rotate
#define CANVAS_BUF_SIZE                                                                             \
    LV_CANVAS_BUF_SIZE(CANVAS_SIZE, CANVAS_SIZE, LV_COLOR_FORMAT_GET_BPP(CANVAS_COLOR_FORMAT),     \
                       LV_DRAW_BUF_STRIDE_ALIGN)

/*
 * The duck art spans two sections. It is authored upright (portrait), drawn into
 * a DUCK_NAT_W x DUCK_NAT_H canvas, then rotated 270 to DUCK_SHOW_W x DUCK_SHOW_H
 * for display (same trick as the 68x68 sections, just taller).
 */
#define DUCK_NAT_W 68
#define DUCK_NAT_H 136
#define DUCK_SHOW_W DUCK_NAT_H
#define DUCK_SHOW_H DUCK_NAT_W
#define DUCK_DRAW_BUF_SIZE                                                                          \
    LV_CANVAS_BUF_SIZE(DUCK_NAT_W, DUCK_NAT_H, LV_COLOR_FORMAT_GET_BPP(CANVAS_COLOR_FORMAT),       \
                       LV_DRAW_BUF_STRIDE_ALIGN)
#define DUCK_SHOW_BUF_SIZE                                                                          \
    LV_CANVAS_BUF_SIZE(DUCK_SHOW_W, DUCK_SHOW_H, LV_COLOR_FORMAT_GET_BPP(CANVAS_COLOR_FORMAT),     \
                       LV_DRAW_BUF_STRIDE_ALIGN)

#define LVGL_BACKGROUND                                                                            \
    IS_ENABLED(CONFIG_NICE_DUCK_WIDGET_INVERTED) ? lv_color_black() : lv_color_white()
#define LVGL_FOREGROUND                                                                            \
    IS_ENABLED(CONFIG_NICE_DUCK_WIDGET_INVERTED) ? lv_color_white() : lv_color_black()

/*
 * One aggregate struct holding every value the screen can show. Each
 * per-parameter widget writes its slice of this struct, then asks the relevant
 * canvas to redraw. Add a field here when you add a new parameter.
 */
struct status_state {
    // Battery
    uint8_t battery;           // central (left) half, 0..100
    bool charging;             // USB power present on the central
    uint8_t peripheral_battery; // right half, 0..100 (polled)

    // Bluetooth / output
    struct zmk_endpoint_instance selected_endpoint;
    int active_profile_index;
    bool active_profile_connected;
    bool active_profile_bonded;
    bool profiles_connected[NICEVIEW_PROFILE_COUNT];
    bool profiles_bonded[NICEVIEW_PROFILE_COUNT];

    // Split link to the right half
    bool peripheral_connected;

    // Layer
    uint8_t layer_index;
    const char *layer_label;

    // HID LED indicators (from the host)
    bool caps_lock;
    bool num_lock;
    bool scroll_lock;

    // Typing speed
    uint8_t wpm;

    // Duck animation frame counter (advanced by the animation timer; draw_middle
    // takes it modulo the frame count).
    uint8_t anim_phase;
};

void rotate_canvas(lv_obj_t *canvas);

/*
 * Rotate a freshly-drawn natural-orientation buffer 270 degrees into a separate
 * display buffer, then point the canvas at the result. Use this when a canvas is
 * drawn at one size but displayed rotated at the swapped size (e.g. art that
 * spans multiple sections); for an in-place square rotate use rotate_canvas().
 *
 * src holds a src_w x src_h image; dst receives the src_h x src_w rotation and
 * becomes the canvas's live buffer. dst must be at least
 * LV_CANVAS_BUF_SIZE(src_h, src_w, ...) bytes.
 */
void rotate_canvas_into(lv_obj_t *canvas, uint8_t *src, uint8_t *dst, lv_coord_t src_w,
                        lv_coord_t src_h);

/* Draws a battery glyph at (x, y) with the given level and charging bolt. */
void draw_battery(lv_obj_t *canvas, lv_coord_t x, lv_coord_t y, uint8_t level, bool charging);

void init_label_dsc(lv_draw_label_dsc_t *label_dsc, lv_color_t color, const lv_font_t *font,
                    lv_text_align_t align);
void init_rect_dsc(lv_draw_rect_dsc_t *rect_dsc, lv_color_t bg_color);
void init_line_dsc(lv_draw_line_dsc_t *line_dsc, lv_color_t color, uint8_t width);
void init_arc_dsc(lv_draw_arc_dsc_t *arc_dsc, lv_color_t color, uint8_t width);

void canvas_draw_line(lv_obj_t *canvas, const lv_point_t points[], uint32_t point_cnt,
                      lv_draw_line_dsc_t *draw_dsc);
void canvas_draw_rect(lv_obj_t *canvas, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h,
                      lv_draw_rect_dsc_t *draw_dsc);
void canvas_draw_arc(lv_obj_t *canvas, lv_coord_t x, lv_coord_t y, lv_coord_t r,
                     int32_t start_angle, int32_t end_angle, lv_draw_arc_dsc_t *draw_dsc);
void canvas_draw_text(lv_obj_t *canvas, lv_coord_t x, lv_coord_t y, lv_coord_t max_w,
                      lv_draw_label_dsc_t *draw_dsc, const char *txt);
void canvas_draw_img(lv_obj_t *canvas, lv_coord_t x, lv_coord_t y, const lv_image_dsc_t *src,
                     lv_draw_image_dsc_t *draw_dsc);
