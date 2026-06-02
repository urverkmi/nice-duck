/*
 * Typing speed (words per minute).
 *
 * Parameter:   wpm
 * ZMK API:     zmk_wpm_get_state() -> the current WPM as a number
 * Events:      zmk_wpm_state_changed
 * Drawn on:    bottom canvas (status.c -> draw_bottom)
 *
 * Requires CONFIG_ZMK_WPM=y.
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/wpm.h>
#include <zmk/events/wpm_state_changed.h>

#include "status.h"

struct wpm_status_state {
    uint8_t wpm;
};

static void set_wpm_status(struct zmk_widget_status *widget, struct wpm_status_state state) {
    widget->state.wpm = state.wpm;
    draw_bottom(widget->obj, &widget->state);
}

static void wpm_status_update_cb(struct wpm_status_state state) {
    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&nice_duck_widgets, widget, node) { set_wpm_status(widget, state); }
}

static struct wpm_status_state wpm_status_get_state(const zmk_event_t *eh) {
    // The current words-per-minute, as a plain number (0..255).
    return (struct wpm_status_state){.wpm = zmk_wpm_get_state()};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_wpm_status, struct wpm_status_state, wpm_status_update_cb,
                            wpm_status_get_state)
ZMK_SUBSCRIPTION(widget_wpm_status, zmk_wpm_state_changed);

void nice_duck_wpm_status_init(void) { widget_wpm_status_init(); }
