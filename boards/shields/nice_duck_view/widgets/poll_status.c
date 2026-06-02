/*
 * Polled value: the peripheral (right half) battery level.
 *
 * ZMK does NOT raise an event when the peripheral battery changes, so there is
 * nothing to subscribe to. We re-read it on a timer on the display work queue
 * and redraw. Bump POLL_INTERVAL down for snappier updates, up to save power.
 *
 * Parameter:   peripheral_battery
 * ZMK API:     zmk_split_central_get_peripheral_battery_level()
 * Drawn on:    top canvas (status.c -> draw_top)
 *
 * Requires CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING=y.
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>

#include <zmk/display.h>

#include "status.h"

#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
#include <zmk/split/central.h>
#endif

#define POLL_INTERVAL K_SECONDS(2)

static struct k_work_delayable poll_work;

static void poll_work_cb(struct k_work *work) {
    uint8_t peripheral_battery = 0;

#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
    // source 0 == the first peripheral (the right half on a two-part split).
    zmk_split_central_get_peripheral_battery_level(0, &peripheral_battery);
#endif

    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&nice_duck_widgets, widget, node) {
        widget->state.peripheral_battery = peripheral_battery;
        draw_top(widget->obj, &widget->state);
    }

    k_work_reschedule_for_queue(zmk_display_work_q(), &poll_work, POLL_INTERVAL);
}

void nice_duck_poll_status_init(void) {
    k_work_init_delayable(&poll_work, poll_work_cb);
    k_work_reschedule_for_queue(zmk_display_work_q(), &poll_work, POLL_INTERVAL);
}
