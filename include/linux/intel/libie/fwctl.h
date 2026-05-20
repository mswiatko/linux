/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2026, Intel Corporation. */

#ifndef _LIBIE_FWCTL_H_
#define _LIBIE_FWCTL_H_
#include <uapi/fwctl/ieth.h>

struct libie_ieth_dev;

struct libie_fwctl {
	int (*send)(struct libie_ieth_dev *ieth, void *desc, size_t desc_len,
		    void *in, size_t in_len, size_t out_len);
};

#endif /* _LIBIE_FWCTL_H_ */
