// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2026, Intel Corporation
 */
#include <linux/auxiliary_bus.h>
#include <linux/fwctl.h>
#include <linux/intel/libie/dev.h>
#include <uapi/fwctl/fwctl.h>
#include <uapi/fwctl/ieth.h>

struct iethctl_uctx {
	struct fwctl_uctx uctx;
	u32 uctx_caps;
};

struct iethctl_dev {
	struct fwctl_device fwctl;
	struct libie_ieth_dev *ieth_dev;
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

static bool iethctl_validate_rpc(const struct fwctl_rpc_ieth *rpc,
				 enum fwctl_rpc_scope scope)
{
	return false;
}

static void *iethctl_fw_rpc(struct fwctl_uctx *uctx, enum fwctl_rpc_scope scope,
			    void *rpc_in, size_t in_len, size_t *out_len)
{
	struct iethctl_dev *ctldev =
		container_of(uctx->fwctl, struct iethctl_dev, fwctl);
	struct libie_ieth_dev *ieth_dev = ctldev->ieth_dev;
	struct fwctl_rpc_ieth *rpc = rpc_in;
	void *buff = NULL, *desc = NULL;
	size_t buff_size;
	int ret;

	if (!iethctl_validate_rpc(rpc_in, scope))
		return ERR_PTR(-EPERM);

	buff_size = max(in_len, *out_len);
	if (buff_size) {
		buff = kzalloc(buff_size, GFP_KERNEL);
		if (!buff)
			return ERR_PTR(-ENOMEM);
	}

	if (in_len) {
		if (copy_from_user(buff, u64_to_user_ptr(rpc->payload),
				   in_len)) {
			ret = -EFAULT;
			goto free_buff;
		}
	}

	if (rpc->desc_len) {
		desc = kzalloc(rpc->desc_len, GFP_KERNEL);
		if (!desc) {
			ret = -ENOMEM;
			goto free_buff;
		}

		if (copy_from_user(desc, u64_to_user_ptr(rpc->desc),
				   rpc->desc_len)) {
			ret = -EFAULT;
			goto free_desc;
		}
	}

	ret = ieth_dev->fwctl.send(ieth_dev, desc, rpc->desc_len, buff, in_len,
				   buff_size);
	if (ret)
		goto free_desc;

	if (*out_len) {
		if (copy_to_user(u64_to_user_ptr(rpc->payload), buff,
				 *out_len)) {
			ret = -EFAULT;
			goto free_desc;
		}
	}

	if (rpc->desc_len) {
		if (copy_to_user(u64_to_user_ptr(rpc->desc), desc,
				 rpc->desc_len)) {
			ret = -EFAULT;
			goto free_desc;
		}
	}

free_desc:
	if (rpc->desc_len)
		kfree(desc);
free_buff:
	if (buff_size)
		kfree(buff);

	if (ret)
		return ERR_PTR(ret);

	return rpc;
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
	struct libie_ieth_dev *ieth_dev =
		container_of(adev, struct libie_ieth_dev, aux);
	int ret;

	if (!ctldev)
		return -ENOMEM;

	ctldev->ieth_dev = ieth_dev;

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
