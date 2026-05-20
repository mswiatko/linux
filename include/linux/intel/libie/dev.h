/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2026, Intel Corporation. */

#ifndef _LIBIE_DEV_H_
#define _LIBIE_DEV_H_
#include <linux/auxiliary_bus.h>
#include <linux/intel/libie/fwctl.h>

enum libie_ieth_type {
    LIBIE_IETH_ICE,
    LIBIE_IETH_IXD,
};

struct libie_ieth_dev;

struct libie_ieth_dev {
    struct auxiliary_device aux;
    enum libie_ieth_type type;

    struct libie_fwctl fwctl;
};

#endif /* _LIBIE_DEV_H_ */
