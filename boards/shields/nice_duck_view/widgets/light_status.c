/*
 * Polled values: "light" (RGB underglow + backlight) and the peripheral (right
 * half) battery level.
 *
 * Unlike battery/BLE/HID, ZMK does NOT raise an event when underglow/backlight
 * toggles or when the peripheral battery changes, so there is nothing to
 * subscribe to. Instead we re-read them on a timer on the display work queue and
 * redraw. Bump POLL_INTERVAL down for snappier updates, up to save power.
 *
 * Parameters:  rgb_on/brightness, backlight_on/brightness, peripheral_battery
 * ZMK API:     zmk_rgb_underglow_get_state(), zmk_rgb_underglow_calc_brt(0),
 *              zmk_backlight_is_on(), zmk_backlight_get_brt(),
 *              zmk_split_central_get_peripheral_battery_level()
 * Drawn on:    top canvas (peripheral battery) + bottom canvas (light)
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>

#include <zmk/display.h>

#include "status.h"

#if IS_ENABLED(CONFIG_ZMK_RGB_UNDERGLOW)
#include <zmk/rgb_underglow.h>
#endif
#if IS_ENABLED(CONFIG_ZMK_BACKLIGHT)
#include <zmk/backlight.h>
#endif
#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
#include <zmk/split/central.h>
#endif

#define POLL_INTERVAL K_SECONDS(2)

static struct k_work_delayable poll_work;

static void poll_work_cb(struct k_work *work) {
    bool rgb_on = false;
    uint8_t rgb_brightness = 0;
    bool bl_on = false;
    uint8_t bl_brightness = 0;
    uint8_t peripheral_battery = 0;

#if IS_ENABLED(CONFIG_ZMK_RGB_UNDERGLOW)
    zmk_rgb_underglow_get_state(&rgb_on);
    // No direct brightness getter; calc_brt(0) returns the current value without
    // applying any change.
    rgb_brightness = zmk_rgb_underglow_calc_brt(0).b;
#endif
#if IS_ENABLED(CONFIG_ZMK_BACKLIGHT)
    bl_on = zmk_backlight_is_on();
    bl_brightness = zmk_backlight_get_brt();
#endif
#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
    // source 0 == the first peripheral (the right half on a two-part split).
    zmk_split_central_get_peripheral_battery_level(0, &peripheral_battery);
#endif

    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&nice_duck_widgets, widget, node) {
        widget->state.rgb_on = rgb_on;
        widget->state.rgb_brightness = rgb_brightness;
        widget->state.backlight_on = bl_on;
        widget->state.backlight_brightness = bl_brightness;
        widget->state.peripheral_battery = peripheral_battery;
        draw_top(widget->obj, &widget->state);
        draw_bottom(widget->obj, &widget->state);
    }

    k_work_reschedule_for_queue(zmk_display_work_q(), &poll_work, POLL_INTERVAL);
}

void nice_duck_light_status_init(void) {
    k_work_init_delayable(&poll_work, poll_work_cb);
    k_work_reschedule_for_queue(zmk_display_work_q(), &poll_work, POLL_INTERVAL);
}
