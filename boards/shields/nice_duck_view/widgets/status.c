/*
 * Status widget: owns the three rotated canvases, the shared widget registry,
 * and the canvas redraw routines. Per-parameter state extraction lives in the
 * sibling widgets/*_status.c files.
 *
 * Canvas layout (each is a 68x68 square, drawn naturally then rotated 270):
 *   top    (child 0): central battery, peripheral battery, BLE/USB symbol
 *   middle (child 1): Bluetooth profile circles + right-half link dot
 *   bottom (child 2): active layer, Caps Lock, WPM
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include "status.h"

#include "../assets/duck_idle.h"
#include "../assets/layer_1.h"
#include "../assets/layer_2.h"
#include "../assets/layer_3.h"
#include "../assets/layer_4.h"
#include "../assets/layer_5.h"

sys_slist_t nice_duck_widgets = SYS_SLIST_STATIC_INIT(&nice_duck_widgets);

/*
 * Display buffer for the rotated duck (136x68). File-static because there is
 * only ever one status widget instance (see registry comment in status.h), and
 * draw_middle only receives the obj, not the widget struct.
 */
static uint8_t duck_show_buf[DUCK_SHOW_BUF_SIZE];

/*
 * Duck animation frames, played in order on a loop by the animation timer (see
 * widgets/duck_status.c). Each frame must be authored at DUCK_NAT_W x DUCK_NAT_H
 * (portrait). Add your exported frames here in playback order; with a single
 * frame the duck is simply static. Declare each with its own header like
 * duck_idle.h and #include it below.
 */
static const lv_image_dsc_t *const duck_frames[] = {
    &duck_idle,
    // &duck_up, &duck_down, ...  <- add bob frames here
};
#define DUCK_FRAME_COUNT (sizeof(duck_frames) / sizeof(duck_frames[0]))

/*
 * Frames shown when BLE is disconnected (selected by draw_middle). Same
 * authoring rules as duck_frames (DUCK_NAT_W x DUCK_NAT_H, in playback order).
 * Placeholder reuses duck_idle so this compiles today; swap in your real offline
 * art and #include its header above. Must have at least one entry.
 */
static const lv_image_dsc_t *const duck_offline_frames[] = {
    &duck_idle, // TODO: replace with &duck_offline_0, &duck_offline_1, ...
};
#define DUCK_OFFLINE_FRAME_COUNT (sizeof(duck_offline_frames) / sizeof(duck_offline_frames[0]))

/*
 * Per-layer hat overlays (RGB565A8 with alpha, authored full-canvas at
 * DUCK_NAT_W x DUCK_NAT_H). draw_middle composites these over the duck, so only
 * the hat's opaque pixels show. Indexed by state->layer_index: 0 is the base
 * layer and wears no hat (bare duck); 1..5 map to layer_1..layer_5. Set any
 * entry to NULL to show the bare duck on that layer.
 */
static const lv_image_dsc_t *const layer_hats[] = {
    NULL,     // 0: base layer, no hat
    &layer_1, // 1
    &layer_2, // 2
    &layer_3, // 3
    &layer_4, // 4
    &layer_5, // 5
};
#define LAYER_HAT_COUNT (sizeof(layer_hats) / sizeof(layer_hats[0]))

/* ------------------------------------------------------------------ top ---- */
void draw_top(lv_obj_t *widget, const struct status_state *state) {
    lv_obj_t *canvas = lv_obj_get_child(widget, 0);
    if (!canvas) {
        return;
    }

    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_16, LV_TEXT_ALIGN_RIGHT);

    lv_canvas_fill_bg(canvas, LVGL_BACKGROUND, LV_OPA_COVER);

    // Central (left half) battery only.
    draw_battery(canvas, 0, 2, state->battery, state->charging);

    // Output / connection symbol for the host link.
    // char output_text[10] = {};
    // switch (state->selected_endpoint.transport) {
    // case ZMK_TRANSPORT_USB:
    //     strcat(output_text, LV_SYMBOL_USB);
    //     break;
    // case ZMK_TRANSPORT_BLE:
    //     if (state->active_profile_bonded) {
    //         strcat(output_text, state->active_profile_connected ? LV_SYMBOL_WIFI : LV_SYMBOL_CLOSE);
    //     } else {
    //         strcat(output_text, LV_SYMBOL_SETTINGS);
    //     }
    //     break;
    // }
    // canvas_draw_text(canvas, 0, 20, CANVAS_SIZE, &label_dsc, output_text);

    rotate_canvas(canvas);
}

/* --------------------------------------------------------------- middle ---- */
/*
 * The duck art. It is authored upright at DUCK_NAT_W x DUCK_NAT_H (68x136), so
 * we point the canvas at a natural-orientation scratch buffer, draw the current
 * animation frame into it, rotate 270 into the persistent show buffer, then
 * point the canvas there. state->anim_phase selects the frame.
 */
void draw_middle(lv_obj_t *widget, const struct status_state *state) {
    lv_obj_t *canvas = lv_obj_get_child(widget, 1);
    if (!canvas) {
        return;
    }

    static uint8_t duck_draw_buf[DUCK_DRAW_BUF_SIZE];

    // Pick the frame set: animated duck when BLE is connected, offline set when not.
    bool ble_connected = state->selected_endpoint.transport == ZMK_TRANSPORT_BLE &&
                         state->active_profile_connected;
    const lv_image_dsc_t *const *frames = ble_connected ? duck_frames : duck_offline_frames;
    size_t count = ble_connected ? DUCK_FRAME_COUNT : DUCK_OFFLINE_FRAME_COUNT;
    const lv_image_dsc_t *frame = frames[state->anim_phase % count];

    

    // Draw the current duck frame upright into the natural-orientation buffer.
    lv_canvas_set_buffer(canvas, duck_draw_buf, DUCK_NAT_W, DUCK_NAT_H, CANVAS_COLOR_FORMAT);
    lv_canvas_fill_bg(canvas, LVGL_BACKGROUND, LV_OPA_COVER);

    lv_draw_image_dsc_t img_dsc;
    lv_draw_image_dsc_init(&img_dsc);
    canvas_draw_img(canvas, 0, 0, frame, &img_dsc);

    // Layer hat overlay: the RGB565A8 alpha composites only the hat pixels over
    // the duck. Base layer (0) and any NULL entry leave the duck bare. When the
    // duck starts bobbing, shift the hat by the same per-frame head offset here
    // (e.g. hat_dy = duck_bob_offset[state->anim_phase % DUCK_FRAME_COUNT]) so it
    // rides on the head instead of detaching.
    if (state->layer_index < LAYER_HAT_COUNT && layer_hats[state->layer_index] != NULL) {
        int hat_dy = 0;
        canvas_draw_img(canvas, 0, hat_dy, layer_hats[state->layer_index], &img_dsc);
    }

    // Rotate 270 from the natural buffer into the show buffer (68x136 -> 136x68)
    // and hand the rotated result to the canvas.
    rotate_canvas_into(canvas, duck_draw_buf, duck_show_buf, DUCK_NAT_W, DUCK_NAT_H);
}

/* --------------------------------------------------------------- bottom ---- */
void draw_bottom(lv_obj_t *widget, const struct status_state *state) {
    lv_obj_t *canvas = lv_obj_get_child(widget, 2);
    if (!canvas) {
        return; // bottom section removed while the duck spans two sections
    }

    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_14, LV_TEXT_ALIGN_CENTER);

    lv_canvas_fill_bg(canvas, LVGL_BACKGROUND, LV_OPA_COVER);

    // Active layer.
    // char layer_text[16] = {};
    // if (state->layer_label == NULL || strlen(state->layer_label) == 0) {
    //     snprintf(layer_text, sizeof(layer_text), "LAYER %i", state->layer_index);
    // } else {
    //     snprintf(layer_text, sizeof(layer_text), "%s", state->layer_label);
    // }
    // canvas_draw_text(canvas, 0, 2, 68, &label_dsc, layer_text);

    // Caps Lock indicator.
    // if (state->caps_lock) {
    //     canvas_draw_text(canvas, 0, 24, 68, &label_dsc, "CAPS");
    // }

    // Words per minute.
    // char wpm_text[12] = {};
    // snprintf(wpm_text, sizeof(wpm_text), "%d WPM", state->wpm);
    // canvas_draw_text(canvas, 0, 46, 68, &label_dsc, wpm_text);

    rotate_canvas(canvas);
}

/* ----------------------------------------------------------------- init ---- */
int zmk_widget_status_init(struct zmk_widget_status *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, 160, 68);

    // Strip the default-theme chrome so the canvases fill the panel exactly:
    // no border, no padding/radius, and no scrollbar from content overflow.
    lv_obj_set_style_border_width(widget->obj, 0, 0);
    lv_obj_set_style_radius(widget->obj, 0, 0);
    lv_obj_set_style_pad_all(widget->obj, 0, 0);
    lv_obj_remove_flag(widget->obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(widget->obj, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *top = lv_canvas_create(widget->obj);
    lv_obj_align(top, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_canvas_set_buffer(top, widget->cbuf, CANVAS_SIZE, CANVAS_SIZE, CANVAS_COLOR_FORMAT);

    // The duck spans the left of the screen (136x68 after rotation), drawn from
    // the shared show buffer. draw_middle re-points and re-sizes it each redraw.
    lv_obj_t *middle = lv_canvas_create(widget->obj);
    lv_obj_align(middle, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_canvas_set_buffer(middle, duck_show_buf, DUCK_SHOW_W, DUCK_SHOW_H, CANVAS_COLOR_FORMAT);

    sys_slist_append(&nice_duck_widgets, &widget->node);

    // Bring up every per-parameter widget. Comment one out to drop a feature.
    nice_duck_battery_status_init();
    nice_duck_output_status_init();
    nice_duck_peripheral_status_init();
    nice_duck_hid_status_init();
    nice_duck_layer_status_init();
    nice_duck_wpm_status_init();
    nice_duck_poll_status_init();
    nice_duck_duck_status_init();

    return 0;
}

lv_obj_t *zmk_widget_status_obj(struct zmk_widget_status *widget) { return widget->obj; }
