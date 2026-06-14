/*
 * Active layer (highest active layer index + its optional label).
 *
 * Parameter:   current layer
 * ZMK API:     zmk_keymap_highest_layer_active(), zmk_keymap_layer_name()
 * Events:      zmk_layer_state_changed
 * Drawn on:    bottom canvas (status.c -> draw_bottom)
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/keymap.h>

#include "status.h"

struct layer_status_state {
    zmk_keymap_layer_index_t index;
    const char *label;
};

static void set_layer_status(struct zmk_widget_status *widget, struct layer_status_state state) {
    widget->state.layer_index = state.index;
    widget->state.layer_label = state.label;
    draw_middle(widget->obj, &widget->state); // layer drives the duck's hat
}

static void layer_status_update_cb(struct layer_status_state state) {
    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&nice_duck_widgets, widget, node) {
        set_layer_status(widget, state);
    }
}

static struct layer_status_state layer_status_get_state(const zmk_event_t *eh) {
    zmk_keymap_layer_index_t index = zmk_keymap_highest_layer_active();
    return (struct layer_status_state){
        .index = index,
        .label = zmk_keymap_layer_name(zmk_keymap_layer_index_to_id(index)),
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_layer_status, struct layer_status_state, layer_status_update_cb,
                            layer_status_get_state)
ZMK_SUBSCRIPTION(widget_layer_status, zmk_layer_state_changed);

void nice_duck_layer_status_init(void) { widget_layer_status_init(); }
