/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Copyright (c) 2026, Intel Corporation
 */

#ifndef _UAPI_FWCTL_IETH_H_
#define _UAPI_FWCTL_IETH_H_

#include <linux/types.h>

enum fwctl_ieth_commands {
	FWCTL_IETH_QUERY_COMMANDS = 0,
	FWCTL_IETH_SEND_COMMANDS,
};

/**
 * struct fwctl_info_ieth - ioctl(FWCTL_INFO) out_device_data
 * @uctx_caps: The command capabilities driver accepts.
 *
 * Return basic information about the FW interface available.
 */
struct fwctl_info_ieth {
	__u32 uctx_caps;
};

/**
 * struct fwctl_rpc_ieth - fwctl message exchange structure
 * @opcode: message opcode
 * @flags: configuration flags
 * @desc_len: length of descriptor
 * @payload_len: length of payload
 * @desc: address of descriptor buffer
 * @payload: address of data buffer
 */
struct fwctl_rpc_ieth {
	__u32 opcode;
	__u32 desc_len;
	__u32 payload_len;
	__aligned_u64 desc;
	__aligned_u64 payload;
};
#endif
