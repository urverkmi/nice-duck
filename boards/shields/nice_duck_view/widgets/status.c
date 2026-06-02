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

sys_slist_t nice_duck_widgets = SYS_SLIST_STATIC_INIT(&nice_duck_widgets);

/* ------------------------------------------------------------------ top ---- */
void draw_top(lv_obj_t *widget, const struct status_state *state) {
    lv_obj_t *canvas = lv_obj_get_child(widget, 0);

    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_16, LV_TEXT_ALIGN_RIGHT);

    lv_canvas_fill_bg(canvas, LVGL_BACKGROUND, LV_OPA_COVER);

    // Central (left) battery, then the right-half battery just below it.
    draw_battery(canvas, 0, 2, state->battery, state->charging);
    draw_battery(canvas, 0, 18, state->peripheral_battery, false);

    // Output / connection symbol for the host link.
    char output_text[10] = {};
    switch (state->selected_endpoint.transport) {
    case ZMK_TRANSPORT_USB:
        strcat(output_text, LV_SYMBOL_USB);
        break;
    case ZMK_TRANSPORT_BLE:
        if (state->active_profile_bonded) {
            strcat(output_text, state->active_profile_connected ? LV_SYMBOL_WIFI : LV_SYMBOL_CLOSE);
        } else {
            strcat(output_text, LV_SYMBOL_SETTINGS);
        }
        break;
    }
    canvas_draw_text(canvas, 0, 36, CANVAS_SIZE, &label_dsc, output_text);

    rotate_canvas(canvas);
}

/* --------------------------------------------------------------- middle ---- */
void draw_middle(lv_obj_t *widget, const struct status_state *state) {
    lv_obj_t *canvas = lv_obj_get_child(widget, 1);

    lv_draw_arc_dsc_t arc_dsc;
    init_arc_dsc(&arc_dsc, LVGL_FOREGROUND, 2);
    lv_draw_arc_dsc_t arc_dsc_filled;
    init_arc_dsc(&arc_dsc_filled, LVGL_FOREGROUND, 9);
    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER);
    lv_draw_label_dsc_t label_dsc_black;
    init_label_dsc(&label_dsc_black, LVGL_BACKGROUND, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER);

    lv_canvas_fill_bg(canvas, LVGL_BACKGROUND, LV_OPA_COVER);

    // Bluetooth profile circles: filled ring = connected, dashed = bonded only.
    int circle_offsets[NICEVIEW_PROFILE_COUNT][2] = {
        {13, 13}, {55, 13}, {34, 34}, {13, 55}, {55, 55},
    };
    for (int i = 0; i < NICEVIEW_PROFILE_COUNT; i++) {
        bool selected = i == state->active_profile_index;

        if (state->profiles_connected[i]) {
            canvas_draw_arc(canvas, circle_offsets[i][0], circle_offsets[i][1], 13, 0, 360,
                            &arc_dsc);
        } else if (state->profiles_bonded[i]) {
            const int segments = 8;
            const int gap = 20;
            for (int j = 0; j < segments; ++j) {
                canvas_draw_arc(canvas, circle_offsets[i][0], circle_offsets[i][1], 13,
                                360. / segments * j + gap / 2.0,
                                360. / segments * (j + 1) - gap / 2.0, &arc_dsc);
            }
        }

        if (selected) {
            canvas_draw_arc(canvas, circle_offsets[i][0], circle_offsets[i][1], 9, 0, 359,
                            &arc_dsc_filled);
        }

        char label[2];
        snprintf(label, sizeof(label), "%d", i + 1);
        canvas_draw_text(canvas, circle_offsets[i][0] - 8, circle_offsets[i][1] - 10, 16,
                         (selected ? &label_dsc_black : &label_dsc), label);
    }

    // Right-half link: filled dot at the centre when connected, outline when not.
    if (state->peripheral_connected) {
        canvas_draw_arc(canvas, 34, 34, 3, 0, 359, &arc_dsc_filled);
    } else {
        canvas_draw_arc(canvas, 34, 34, 4, 0, 360, &arc_dsc);
    }

    rotate_canvas(canvas);
}

/* --------------------------------------------------------------- bottom ---- */
void draw_bottom(lv_obj_t *widget, const struct status_state *state) {
    lv_obj_t *canvas = lv_obj_get_child(widget, 2);

    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_14, LV_TEXT_ALIGN_CENTER);

    lv_canvas_fill_bg(canvas, LVGL_BACKGROUND, LV_OPA_COVER);

    // Active layer.
    char layer_text[16] = {};
    if (state->layer_label == NULL || strlen(state->layer_label) == 0) {
        snprintf(layer_text, sizeof(layer_text), "LAYER %i", state->layer_index);
    } else {
        snprintf(layer_text, sizeof(layer_text), "%s", state->layer_label);
    }
    canvas_draw_text(canvas, 0, 2, 68, &label_dsc, layer_text);

    // Caps Lock indicator.
    if (state->caps_lock) {
        canvas_draw_text(canvas, 0, 24, 68, &label_dsc, "CAPS");
    }

    // Words per minute.
    char wpm_text[12] = {};
    snprintf(wpm_text, sizeof(wpm_text), "%d WPM", state->wpm);
    canvas_draw_text(canvas, 0, 46, 68, &label_dsc, wpm_text);

    rotate_canvas(canvas);
}

/* ----------------------------------------------------------------- init ---- */
int zmk_widget_status_init(struct zmk_widget_status *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, 160, 68);

    lv_obj_t *top = lv_canvas_create(widget->obj);
    lv_obj_align(top, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_canvas_set_buffer(top, widget->cbuf, CANVAS_SIZE, CANVAS_SIZE, CANVAS_COLOR_FORMAT);

    lv_obj_t *middle = lv_canvas_create(widget->obj);
    lv_obj_align(middle, LV_ALIGN_TOP_LEFT, 24, 0);
    lv_canvas_set_buffer(middle, widget->cbuf2, CANVAS_SIZE, CANVAS_SIZE, CANVAS_COLOR_FORMAT);

    lv_obj_t *bottom = lv_canvas_create(widget->obj);
    lv_obj_align(bottom, LV_ALIGN_TOP_LEFT, -44, 0);
    lv_canvas_set_buffer(bottom, widget->cbuf3, CANVAS_SIZE, CANVAS_SIZE, CANVAS_COLOR_FORMAT);

    sys_slist_append(&nice_duck_widgets, &widget->node);

    // Bring up every per-parameter widget. Comment one out to drop a feature.
    nice_duck_battery_status_init();
    nice_duck_output_status_init();
    nice_duck_peripheral_status_init();
    nice_duck_hid_status_init();
    nice_duck_layer_status_init();
    nice_duck_wpm_status_init();
    nice_duck_poll_status_init();

    return 0;
}

lv_obj_t *zmk_widget_status_obj(struct zmk_widget_status *widget) { return widget->obj; }
