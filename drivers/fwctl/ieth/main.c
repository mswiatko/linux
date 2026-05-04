// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2026, Intel Corporation
 */
#include <linux/auxiliary_bus.h>
#include <linux/fwctl.h>
#include <uapi/fwctl/fwctl.h>
#include <uapi/fwctl/ieth.h>

struct iethctl_uctx {
	struct fwctl_uctx uctx;
	u32 uctx_caps;
};

struct iethctl_dev {
	struct fwctl_device fwctl;
};
DEFINE_FREE(iethctl, struct iethctl_dev *, if (_T) fwctl_put(&_T->fwctl));

static int iethctl_open_uctx(struct fwctl_uctx *uctx)
{
	struct iethctl_uctx *iethctl_uctx =
		container_of(uctx, struct iethctl_uctx, uctx);

	iethctl_uctx->uctx_caps = BIT(FWCTL_IETH_QUERY_COMMANDS) |
				  BIT(FWCTL_IETH_SEND_COMMANDS);
	return 0;
}

static void iethctl_close_uctx(struct fwctl_uctx *uctx)
{
}

static void *iethctl_info(struct fwctl_uctx *uctx, size_t *length)
{
	struct iethctl_uctx *iethctl_uctx =
		container_of(uctx, struct iethctl_uctx, uctx);
	struct fwctl_info_ieth *info;

	info = kzalloc_obj(*info);
	if (!info)
		return ERR_PTR(-ENOMEM);

	info->uctx_caps = iethctl_uctx->uctx_caps;

	*length = sizeof(*info);
	return info;
}

static bool iethctl_validate_rpc(const void *in, enum fwctl_rpc_scope scope)
{
	return false;
}

static void *iethctl_fw_rpc(struct fwctl_uctx *uctx, enum fwctl_rpc_scope scope,
			    void *rpc_in, size_t in_len, size_t *out_len)
{
	if (!iethctl_validate_rpc(rpc_in, scope))
		return ERR_PTR(-EPERM);

	return ERR_PTR(-EOPNOTSUPP);
}

static const struct fwctl_ops iethctl_ops = {
	.uctx_size = sizeof(struct iethctl_uctx),
	.open_uctx = iethctl_open_uctx,
	.close_uctx = iethctl_close_uctx,
	.info = iethctl_info,
	.fw_rpc = iethctl_fw_rpc,
};

static int iethctl_probe(struct auxiliary_device *adev,
			 const struct auxiliary_device_id *id)
{
	struct iethctl_dev *ctldev __free(iethctl) =
		fwctl_alloc_device(&adev->dev, &iethctl_ops, struct iethctl_dev,
				   fwctl);
	int ret;

	if (!ctldev)
		return -ENOMEM;

	ret = fwctl_register(&ctldev->fwctl);
	if (ret)
		return ret;

	auxiliary_set_drvdata(adev, no_free_ptr(ctldev));
	return 0;
}

static void iethctl_remove(struct auxiliary_device *adev)
{
	struct iethctl_dev *ctldev = auxiliary_get_drvdata(adev);

	fwctl_unregister(&ctldev->fwctl);
	fwctl_put(&ctldev->fwctl);
}

static const struct auxiliary_device_id iethctl_id_table[] = {
	{ .name = "ice.fwctl", },
	{ .name = "ixd.fwctl", },
	{}
};
MODULE_DEVICE_TABLE(auxiliary, iethctl_id_table);

static struct auxiliary_driver iethctl_driver = {
	.name = "ieth_fwctl",
	.probe = iethctl_probe,
	.remove = iethctl_remove,
	.id_table = iethctl_id_table,
};

module_auxiliary_driver(iethctl_driver);

MODULE_IMPORT_NS("FWCTL");
MODULE_DESCRIPTION("Intel Ethernet fwctl driver");
MODULE_AUTHOR("Michal Swiatkowski <michal.swiatkowski@linux.intel.com>");
MODULE_LICENSE("GPL");
