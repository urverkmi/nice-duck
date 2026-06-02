/*
 * HID LED indicators from the host: Caps Lock (and Num/Scroll Lock).
 *
 * Parameter:   caps_lock / num_lock / scroll_lock
 * ZMK API:     zmk_hid_indicators_get_current_profile() -> bitmask of the USB
 *              HID keyboard LED report byte.
 * Events:      zmk_hid_indicators_changed (carries the same bitmask)
 * Drawn on:    bottom canvas (status.c -> draw_bottom, the "CAPS" text)
 *
 * Requires CONFIG_ZMK_HID_INDICATORS=y.
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/hid_indicators.h>
#include <zmk/events/hid_indicators_changed.h>

#include "status.h"

// USB HID keyboard LED report bits (Output report, LED usage page).
#define HID_LED_NUM_LOCK BIT(0)
#define HID_LED_CAPS_LOCK BIT(1)
#define HID_LED_SCROLL_LOCK BIT(2)

struct hid_status_state {
    zmk_hid_indicators_t indicators;
};

static void set_hid_status(struct zmk_widget_status *widget, struct hid_status_state state) {
    widget->state.num_lock = state.indicators & HID_LED_NUM_LOCK;
    widget->state.caps_lock = state.indicators & HID_LED_CAPS_LOCK;
    widget->state.scroll_lock = state.indicators & HID_LED_SCROLL_LOCK;
    draw_bottom(widget->obj, &widget->state);
}

static void hid_status_update_cb(struct hid_status_state state) {
    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&nice_duck_widgets, widget, node) { set_hid_status(widget, state); }
}

static struct hid_status_state hid_status_get_state(const zmk_event_t *eh) {
    const struct zmk_hid_indicators_changed *ev = as_zmk_hid_indicators_changed(eh);
    return (struct hid_status_state){
        .indicators = (ev != NULL) ? ev->indicators : zmk_hid_indicators_get_current_profile(),
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_hid_status, struct hid_status_state, hid_status_update_cb,
                            hid_status_get_state)
ZMK_SUBSCRIPTION(widget_hid_status, zmk_hid_indicators_changed);

void nice_duck_hid_status_init(void) { widget_hid_status_init(); }
