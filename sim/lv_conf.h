/*
 * Minimal LVGL config for the nice!duck desktop simulator. Only the options
 * that differ from LVGL's built-in defaults are set here; everything else falls
 * through to lv_conf_internal.h defaults.
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

/* Use the host C library for malloc / string / sprintf so we don't have to
 * size LVGL's builtin memory pool. */
#define LV_USE_STDLIB_MALLOC   LV_STDLIB_CLIB
#define LV_USE_STDLIB_STRING   LV_STDLIB_CLIB
#define LV_USE_STDLIB_SPRINTF  LV_STDLIB_CLIB

/* 32-bit ARGB framebuffer for the SDL window (the shield itself is 1-bit, but
 * the widgets draw black-on-white so it previews identically). */
#define LV_COLOR_DEPTH 32

/* The SDL display + mouse driver used by sim/main.c. Homebrew's SDL2 headers
 * live in include/SDL2/, so include <SDL.h> rather than LVGL's default
 * <SDL2/SDL.h>. */
#define LV_USE_SDL 1
#define LV_SDL_INCLUDE_PATH <SDL.h>

/* Fonts the status widgets reference. */
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/* Quiet by default; flip to 1 if you want LVGL's internal logging. */
#define LV_USE_LOG 0

/* Headless snapshot-to-PNG mode (SIM_PNG=1) needs the snapshot module. */
#define LV_USE_SNAPSHOT 1
