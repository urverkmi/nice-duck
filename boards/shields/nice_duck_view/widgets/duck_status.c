/*
 * Duck animation driver.
 *
 * There is no ZMK event to drive a looping animation, so (like poll_status.c) we
 * run a self-rescheduling timer on the display work queue. Each tick advances
 * the frame counter and redraws the duck (status.c -> draw_middle, which plays
 * the duck_frames[] array in order).
 *
 * Power: while the keyboard is idle/asleep we keep the timer ticking but skip
 * the redraw, so no SPI flush to the memory LCD happens until activity resumes.
 * Drop FRAME_INTERVAL for a smoother loop, raise it to save power.
 *
 * Parameter:   anim_phase
 * Drawn on:    middle canvas (status.c -> draw_middle)
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>

#include <zmk/activity.h>
#include <zmk/display.h>

#include "status.h"

#define FRAME_INTERVAL K_MSEC(250) // ~4 fps: choppy, stop-motion feel

static struct k_work_delayable anim_work;

static void anim_work_cb(struct k_work *work) {
    // Only redraw while the keyboard is active; freeze (no LCD flush) when idle.
    if (zmk_activity_get_state() == ZMK_ACTIVITY_ACTIVE) {
        struct zmk_widget_status *widget;
        SYS_SLIST_FOR_EACH_CONTAINER(&nice_duck_widgets, widget, node) {
            widget->state.anim_phase++;
            draw_middle(widget->obj, &widget->state);
        }
    }

    k_work_reschedule_for_queue(zmk_display_work_q(), &anim_work, FRAME_INTERVAL);
}

void nice_duck_duck_status_init(void) {
    k_work_init_delayable(&anim_work, anim_work_cb);
    k_work_reschedule_for_queue(zmk_display_work_q(), &anim_work, FRAME_INTERVAL);
}
