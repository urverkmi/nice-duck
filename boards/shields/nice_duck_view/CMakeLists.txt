# SPDX-License-Identifier: MIT

if(CONFIG_ZMK_DISPLAY AND CONFIG_NICE_DUCK_WIDGET_STATUS)
  zephyr_library_include_directories(${CMAKE_SOURCE_DIR}/include)

  zephyr_library_sources(custom_status_screen.c)

  # The custom screen is only built for the central (left) half. The right half
  # uses the stock nice_view shield (see build.yaml).
  if(NOT CONFIG_ZMK_SPLIT OR CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    zephyr_library_sources(widgets/util.c)
    zephyr_library_sources(widgets/bolt.c)
    zephyr_library_sources(widgets/status.c)
    zephyr_library_sources(widgets/battery_status.c)
    zephyr_library_sources(widgets/output_status.c)
    zephyr_library_sources(widgets/peripheral_status.c)
    zephyr_library_sources(widgets/hid_status.c)
    zephyr_library_sources(widgets/layer_status.c)
    zephyr_library_sources(widgets/wpm_status.c)
    zephyr_library_sources(widgets/poll_status.c)
    zephyr_library_sources(widgets/duck_status.c)
    zephyr_library_sources(assets/duck_idle.c)
    zephyr_library_sources(assets/layer_1.c)
    zephyr_library_sources(assets/layer_2.c)
    zephyr_library_sources(assets/layer_3.c)
    zephyr_library_sources(assets/layer_4.c)
    zephyr_library_sources(assets/layer_5.c)

    # Your designed bitmaps (see assets/README.md) get added here, e.g.:
    # zephyr_library_sources(assets/example_icon.c)
  endif()
endif()
