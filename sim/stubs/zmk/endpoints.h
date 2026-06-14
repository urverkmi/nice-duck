/*
 * Host stub for <zmk/endpoints.h> — only the transport enum and the endpoint
 * instance struct that the status widgets read.
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

enum zmk_transport {
    ZMK_TRANSPORT_USB,
    ZMK_TRANSPORT_BLE,
};

struct zmk_endpoint_instance {
    enum zmk_transport transport;
    union {
        struct {
            uint8_t index;
        } ble;
    };
};
