# Desktop simulator for the nice!duck status screen.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.16)
project(nice_duck_sim C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(SIM_DIR    ${CMAKE_CURRENT_SOURCE_DIR})
set(SHIELD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../boards/shields/nice_duck_view)

# --- SDL2 (Homebrew) -------------------------------------------------------
list(APPEND CMAKE_PREFIX_PATH /opt/homebrew /usr/local)
find_package(SDL2 REQUIRED)

# --- LVGL ------------------------------------------------------------------
# Point LVGL at our trimmed lv_conf.h and skip the parts we don't need.
set(LV_CONF_PATH ${SIM_DIR}/lv_conf.h CACHE STRING "" FORCE)
set(LV_CONF_BUILD_DISABLE_EXAMPLES ON)
set(LV_CONF_BUILD_DISABLE_DEMOS ON)
set(LV_CONF_BUILD_DISABLE_THORVG_INTERNAL ON)
add_subdirectory(lvgl)

# LVGL globs in src/drivers/sdl/*.c (guarded by LV_USE_SDL), so the lvgl target
# itself needs the SDL2 headers + link.
target_link_libraries(lvgl PUBLIC SDL2::SDL2)

# --- Simulator executable --------------------------------------------------
add_executable(nice_duck_sim
    ${SIM_DIR}/main.c
    ${SHIELD_DIR}/widgets/util.c
    ${SHIELD_DIR}/widgets/bolt.c
    ${SHIELD_DIR}/widgets/status.c
    ${SHIELD_DIR}/assets/duck_idle.c
    ${SHIELD_DIR}/assets/layer_1.c
    ${SHIELD_DIR}/assets/layer_2.c
    ${SHIELD_DIR}/assets/layer_3.c
    ${SHIELD_DIR}/assets/layer_4.c
    ${SHIELD_DIR}/assets/layer_5.c
)

target_include_directories(nice_duck_sim PRIVATE
    ${SIM_DIR}/stubs   # zephyr/* and zmk/* host stubs (searched first)
    ${SHIELD_DIR}      # so "widgets/status.h" resolves
)

target_link_libraries(nice_duck_sim PRIVATE lvgl SDL2::SDL2)
