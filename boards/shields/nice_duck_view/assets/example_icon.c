/*
 * Placeholder converted image: an 8x8 box outline in 1-bit indexed (I1) format.
 *
 * This is what the LVGL image converter spits out — replace it with your own
 * generated file (see assets/README.md). It is NOT compiled by default; add it
 * to the shield CMakeLists.txt to use it.
 *
 * I1 layout: 2 palette entries (4 bytes each) followed by 1 bit per pixel,
 * MSB-first, padded to whole bytes per row.
 *
 * SPDX-License-Identifier: MIT
 */

#include <lvgl.h>

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

static const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST uint8_t example_icon_map[] = {
    0x00, 0x00, 0x00, 0xff, /* index 0: background */
    0xff, 0xff, 0xff, 0xff, /* index 1: foreground */

    0xff, /* ######## */
    0x81, /* #      # */
    0x81, /* #      # */
    0x81, /* #      # */
    0x81, /* #      # */
    0x81, /* #      # */
    0x81, /* #      # */
    0xff, /* ######## */
};

const lv_img_dsc_t example_icon = {
    .header.cf = LV_COLOR_FORMAT_I1,
    .header.w = 8,
    .header.h = 8,
    .data_size = sizeof(example_icon_map),
    .data = example_icon_map,
};
