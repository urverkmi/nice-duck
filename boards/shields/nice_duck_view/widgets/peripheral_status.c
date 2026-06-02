/*
 * Split connection status: is the right (peripheral) half connected to this
 * central half?
 *
 * Parameter:   peripheral_connected
 * ZMK API:     event payload (zmk_split_peripheral_status_changed.connected)
 * Events:      zmk_split_peripheral_status_changed
 * Drawn on:    middle canvas (status.c -> draw_middle, the centre dot)
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/split_peripheral_status_changed.h>

#include "status.h"

struct peripheral_status_state {
    bool connected;
};

static void set_peripheral_status(struct zmk_widget_status *widget,
                                  struct peripheral_status_state state) {
    widget->state.peripheral_connected = state.connected;
    draw_middle(widget->obj, &widget->state);
}

static void peripheral_status_update_cb(struct peripheral_status_state state) {
    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&nice_duck_widgets, widget, node) {
        set_peripheral_status(widget, state);
    }
}

static struct peripheral_status_state peripheral_status_get_state(const zmk_event_t *eh) {
    const struct zmk_split_peripheral_status_changed *ev =
        as_zmk_split_peripheral_status_changed(eh);
    return (struct peripheral_status_state){.connected = (ev != NULL) ? ev->connected : false};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_peripheral_status, struct peripheral_status_state,
                            peripheral_status_update_cb, peripheral_status_get_state)
ZMK_SUBSCRIPTION(widget_peripheral_status, zmk_split_peripheral_status_changed);

void nice_duck_peripheral_status_init(void) { widget_peripheral_status_init(); }
