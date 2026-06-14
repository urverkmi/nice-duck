/*
 * Host stub for <zephyr/kernel.h> — just enough for the nice_duck widgets to
 * compile off-target. Provides the intrusive singly-linked list the widget
 * registry uses, the IS_ENABLED() macro, and the standard fixed-width types.
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

/* --- IS_ENABLED: any undefined CONFIG_* is treated as disabled (0) -------- */
#define _IS_ENABLED1(x)        _IS_ENABLED2(_XXXX##x)
#define _XXXX1                 _YYYY,
#define _IS_ENABLED2(one_or_two) _IS_ENABLED3(one_or_two 1, 0)
#define _IS_ENABLED3(ignore, val, ...) val
#define IS_ENABLED(cfg)        _IS_ENABLED1(cfg)

/* --- sys_slist: a minimal intrusive singly-linked list -------------------- */
typedef struct _snode {
    struct _snode *next;
} sys_snode_t;

typedef struct _slist {
    sys_snode_t *head;
    sys_snode_t *tail;
} sys_slist_t;

#define SYS_SLIST_STATIC_INIT(ptr) {NULL, NULL}

static inline void sys_slist_append(sys_slist_t *list, sys_snode_t *node) {
    node->next = NULL;
    if (list->tail == NULL) {
        list->head = list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }
}

static inline sys_snode_t *sys_slist_peek_head(sys_slist_t *list) { return list->head; }
static inline sys_snode_t *sys_slist_peek_next(sys_snode_t *node) { return node->next; }

#define SYS_SLIST_FOR_EACH_CONTAINER(list, container, member)                                      \
    for (container = (void *)0; 0;)
