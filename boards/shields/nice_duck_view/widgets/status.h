/*
 * The single status widget. It owns three rotated canvases and the shared
 * state; per-parameter widgets (battery_status.c, output_status.c, ...) fill in
 * the state and call the draw_* routines below.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>
#include "util.h"

struct zmk_widget_status {
    sys_snode_t node;
    lv_obj_t *obj;
    uint8_t cbuf[CANVAS_BUF_SIZE];  // top canvas    (child 0)
    uint8_t cbuf2[CANVAS_BUF_SIZE]; // middle canvas (child 1)
    uint8_t cbuf3[CANVAS_BUF_SIZE]; // bottom canvas (child 2)
    struct status_state state;
};

int zmk_widget_status_init(struct zmk_widget_status *widget, lv_obj_t *parent);
lv_obj_t *zmk_widget_status_obj(struct zmk_widget_status *widget);

/*
 * Shared registry of widget instances. Each per-parameter widget iterates this
 * list to update every instance (there is normally just one).
 */
extern sys_slist_t nice_duck_widgets;

/*
 * Canvas redraw routines (defined in status.c). A per-parameter widget calls
 * the canvas(es) its data appears on after updating widget->state.
 *
 *   draw_top    -> central + peripheral battery, BLE/USB output symbol
 *   draw_middle -> Bluetooth profile circles, right-half link dot
 *   draw_bottom -> active layer, Caps Lock, WPM
 */
void draw_top(lv_obj_t *widget, const struct status_state *state);
void draw_middle(lv_obj_t *widget, const struct status_state *state);
void draw_bottom(lv_obj_t *widget, const struct status_state *state);

/* Per-parameter init hooks — each defined in its own widgets/<x>_status.c. */
void nice_duck_battery_status_init(void);
void nice_duck_output_status_init(void);
void nice_duck_peripheral_status_init(void);
void nice_duck_hid_status_init(void);
void nice_duck_layer_status_init(void);
void nice_duck_wpm_status_init(void);
void nice_duck_poll_status_init(void);
void nice_duck_duck_status_init(void);
