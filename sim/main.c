/*
 * Desktop simulator for the nice!duck status screen.
 *
 * It reuses the real widget code (widgets/status.c, util.c, ...) compiled
 * against host stubs for the Zephyr/ZMK headers, creates the same three rotated
 * canvases the firmware does, and cycles through a set of mocked status_states
 * so you can eyeball every screen state without flashing.
 *
 * Controls: states auto-advance every few seconds; the current one is shown in
 * the window title. Close the window or Ctrl+C in the terminal to quit.
 *
 * SPDX-License-Identifier: MIT
 */

#include <lvgl.h>
#include "src/drivers/sdl/lv_sdl_window.h"
#include "src/drivers/sdl/lv_sdl_mouse.h"

#include "widgets/status.h"

#include <stdlib.h>
#include <stdio.h>

/* ---- ZMK per-parameter init hooks: no-ops in the sim. We feed state by hand
 *      instead of wiring up real battery/BLE/HID listeners. -------------------*/
void nice_duck_battery_status_init(void) {}
void nice_duck_output_status_init(void) {}
void nice_duck_peripheral_status_init(void) {}
void nice_duck_hid_status_init(void) {}
void nice_duck_layer_status_init(void) {}
void nice_duck_wpm_status_init(void) {}
void nice_duck_poll_status_init(void) {}
void nice_duck_duck_status_init(void) {} // sim drives the animation via an lv_timer instead

/* ---- Mocked screen states ------------------------------------------------- */
typedef struct {
    const char *name;
    struct status_state state;
} preset_t;

static preset_t presets[] = {
    {
        .name = "USB / charging / full",
        .state = {
            .battery = 92, .charging = true, .peripheral_battery = 80,
            .selected_endpoint = {.transport = ZMK_TRANSPORT_USB},
            .peripheral_connected = true,
            .layer_index = 1, .layer_label = "L1", .wpm = 65,
        },
    },
    {
        .name = "BLE profile 1 connected",
        .state = {
            .battery = 74, .peripheral_battery = 66,
            .selected_endpoint = {.transport = ZMK_TRANSPORT_BLE},
            .active_profile_index = 0,
            .active_profile_connected = true, .active_profile_bonded = true,
            .profiles_connected = {true, false, false, false, false},
            .profiles_bonded = {true, true, true, false, false},
            .peripheral_connected = true,
            .layer_index = 2, .layer_label = "L2", .wpm = 41,
        },
    },
    {
        .name = "BLE bonded but disconnected",
        .state = {
            .battery = 33, .peripheral_battery = 20,
            .selected_endpoint = {.transport = ZMK_TRANSPORT_BLE},
            .active_profile_index = 1,
            .active_profile_connected = false, .active_profile_bonded = true,
            .profiles_bonded = {true, true, false, false, false},
            .peripheral_connected = false,
            .layer_index = 3, .layer_label = "L3", .wpm = 0,
        },
    },
    {
        .name = "Low battery / unbonded",
        .state = {
            .battery = 8, .peripheral_battery = 0,
            .selected_endpoint = {.transport = ZMK_TRANSPORT_BLE},
            .active_profile_index = 2,
            .active_profile_connected = false, .active_profile_bonded = false,
            .peripheral_connected = false,
            .layer_index = 5, .layer_label = "L5", .wpm = 118,
        },
    },
};
#define PRESET_COUNT (sizeof(presets) / sizeof(presets[0]))

static lv_display_t *disp;
static struct zmk_widget_status widget;
static uint32_t cur = 0;

static void render(uint32_t i) {
    const preset_t *p = &presets[i % PRESET_COUNT];
    draw_top(widget.obj, &p->state);
    draw_middle(widget.obj, &p->state);
    draw_bottom(widget.obj, &p->state);
    lv_sdl_window_set_title(disp, p->name);
}

static void cycle_cb(lv_timer_t *t) {
    LV_UNUSED(t);
    cur = (cur + 1) % PRESET_COUNT;
    render(cur);
}

/* Stand-in for the firmware's duck_status.c timer: advance the duck frame. */
static void anim_cb(lv_timer_t *t) {
    LV_UNUSED(t);
    presets[cur].state.anim_phase++;
    draw_middle(widget.obj, &presets[cur].state);
}

/* Dump the live screen (160x68) to a binary PPM we can convert to PNG. */
static void snapshot_ppm(lv_obj_t *screen, const char *path) {
    lv_refr_now(disp);
    lv_draw_buf_t *snap = lv_snapshot_take(screen, LV_COLOR_FORMAT_ARGB8888);
    if (!snap) {
        fprintf(stderr, "snapshot failed for %s\n", path);
        return;
    }
    FILE *f = fopen(path, "wb");
    if (f) {
        const uint32_t w = snap->header.w, h = snap->header.h;
        fprintf(f, "P6\n%u %u\n255\n", w, h);
        for (uint32_t y = 0; y < h; y++) {
            const uint8_t *row = snap->data + (size_t)y * snap->header.stride;
            for (uint32_t x = 0; x < w; x++) {
                const uint8_t *px = row + x * 4; /* ARGB8888 in memory: B,G,R,A */
                fputc(px[2], f);                 /* R */
                fputc(px[1], f);                 /* G */
                fputc(px[0], f);                 /* B */
            }
        }
        fclose(f);
        printf("wrote %s (%ux%u)\n", path, w, h);
    }
    lv_draw_buf_destroy(snap);
}

int main(void) {
    lv_init();

    /* nice!view is 160x68; zoom up so it's comfortable on a desktop monitor. */
    disp = lv_sdl_window_create(160, 68);
    lv_sdl_window_set_zoom(disp, 6);
    lv_sdl_mouse_create();

    lv_obj_t *screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_white(), 0);

    zmk_widget_status_init(&widget, screen);
    lv_obj_align(zmk_widget_status_obj(&widget), LV_ALIGN_TOP_LEFT, 0, 0);

    /* Raw-art preview: draw duck_idle full size (160x68, no rotation) so we can
     * see exactly what the asset contains. */
    if (getenv("SIM_DUCK")) {
        extern const lv_image_dsc_t duck_idle;
        static uint8_t big[LV_CANVAS_BUF_SIZE(160, 68, 8, LV_DRAW_BUF_STRIDE_ALIGN)];
        lv_obj_t *c = lv_canvas_create(screen);
        lv_canvas_set_buffer(c, big, 160, 68, LV_COLOR_FORMAT_L8);
        lv_obj_align(c, LV_ALIGN_TOP_LEFT, 0, 0);
        lv_canvas_fill_bg(c, lv_color_white(), LV_OPA_COVER);
        lv_layer_t layer;
        lv_canvas_init_layer(c, &layer);
        lv_draw_image_dsc_t d;
        lv_draw_image_dsc_init(&d);
        d.src = &duck_idle;
        lv_area_t a = {0, 0, 159, 67};
        lv_draw_image(&layer, &d, &a);
        lv_canvas_finish_layer(c, &layer);
        snapshot_ppm(screen, "duck_raw.ppm");
        return 0;
    }

    /* Headless mode: render each preset, dump a PPM, exit. */
    if (getenv("SIM_PNG")) {
        for (uint32_t i = 0; i < PRESET_COUNT; i++) {
            render(i);
            char path[64];
            snprintf(path, sizeof(path), "preset_%u.ppm", i);
            snapshot_ppm(screen, path);
        }
        return 0;
    }

    render(cur);
    lv_timer_create(cycle_cb, 2500, NULL);
    lv_timer_create(anim_cb, 250, NULL); // ~4 fps, matches duck_status.c FRAME_INTERVAL

    while (1) {
        uint32_t idle = lv_timer_handler();
        if (idle > 100) idle = 100;
        if (idle < 5) idle = 5;
        lv_delay_ms(idle);
    }
    return 0;
}
