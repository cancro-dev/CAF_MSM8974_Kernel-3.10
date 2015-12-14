<<<<<<< HEAD
/* Copyright (c) 2014 - 2015, The Linux Foundation. All rights reserved.
=======
/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
>>>>>>> 5b5616e... Add LG G3 Support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt) "%s:%d " fmt, __func__, __LINE__

#include <linux/module.h>
<<<<<<< HEAD
#include "msm_sd.h"
#include "msm_ois.h"
#include "msm_cci.h"

DEFINE_MSM_MUTEX(msm_ois_mutex);
/*#define MSM_OIS_DEBUG*/
#undef CDBG
#ifdef MSM_OIS_DEBUG
#define CDBG(fmt, args...) pr_err(fmt, ##args)
#else
#define CDBG(fmt, args...) pr_debug(fmt, ##args)
#endif

#define MAX_POLL_COUNT 100

static struct v4l2_file_operations msm_ois_v4l2_subdev_fops;
static int32_t msm_ois_power_up(struct msm_ois_ctrl_t *o_ctrl);
static int32_t msm_ois_power_down(struct msm_ois_ctrl_t *o_ctrl);

static struct i2c_driver msm_ois_i2c_driver;

static int32_t msm_ois_write_settings(struct msm_ois_ctrl_t *o_ctrl,
	uint16_t size, struct reg_settings_ois_t *settings)
{
	int32_t rc = -EFAULT;
	int32_t i = 0;
	struct msm_camera_i2c_seq_reg_array reg_setting;
	CDBG("Enter\n");

	for (i = 0; i < size; i++) {
		switch (settings[i].i2c_operation) {
		case MSM_OIS_WRITE: {
			switch (settings[i].data_type) {
			case MSM_CAMERA_I2C_BYTE_DATA:
			case MSM_CAMERA_I2C_WORD_DATA:
				rc = o_ctrl->i2c_client.i2c_func_tbl->i2c_write(
					&o_ctrl->i2c_client,
					settings[i].reg_addr,
					settings[i].reg_data,
					settings[i].data_type);
				break;
			case MSM_CAMERA_I2C_DWORD_DATA:
				reg_setting.reg_addr = settings[i].reg_addr;
				reg_setting.reg_data[0] = (uint8_t)
					((settings[i].reg_data &
					0xFF000000) >> 24);
				reg_setting.reg_data[1] = (uint8_t)
					((settings[i].reg_data &
					0x00FF0000) >> 16);
				reg_setting.reg_data[2] = (uint8_t)
					((settings[i].reg_data &
					0x0000FF00) >> 8);
				reg_setting.reg_data[3] = (uint8_t)
					(settings[i].reg_data & 0x000000FF);
				reg_setting.reg_data_size = 4;
				rc = o_ctrl->i2c_client.i2c_func_tbl->
					i2c_write_seq(&o_ctrl->i2c_client,
					reg_setting.reg_addr,
					reg_setting.reg_data,
					reg_setting.reg_data_size);
				if (rc < 0)
					return rc;
				break;

			default:
				pr_err("Unsupport data type: %d\n",
					settings[i].data_type);
				break;
			}
		}
			break;

		case MSM_OIS_POLL: {
			int32_t poll_count = 0;
			switch (settings[i].data_type) {
			case MSM_CAMERA_I2C_BYTE_DATA:
			case MSM_CAMERA_I2C_WORD_DATA:
				do {
					rc = o_ctrl->i2c_client.i2c_func_tbl
						->i2c_poll(&o_ctrl->i2c_client,
						settings[i].reg_addr,
						settings[i].reg_data,
						settings[i].data_type);

					if (poll_count++ > MAX_POLL_COUNT) {
						pr_err("MSM_OIS_POLL failed");
						break;
					}
				} while (rc != 0);
				break;

			default:
				pr_err("Unsupport data type: %d\n",
					settings[i].data_type);
				break;
			}
		}
		}

		if (settings[i].delay > 20)
			msleep(settings[i].delay);
		else if (0 != settings[i].delay)
			usleep_range(settings[i].delay * 1000,
				(settings[i].delay * 1000) + 1000);

		if (rc < 0)
			break;
	}

	CDBG("Exit\n");
	return rc;
}

static int32_t msm_ois_vreg_control(struct msm_ois_ctrl_t *o_ctrl,
							int config)
{
	int rc = 0, i, cnt;
	struct msm_ois_vreg *vreg_cfg;

	vreg_cfg = &o_ctrl->vreg_cfg;
	cnt = vreg_cfg->num_vreg;
	if (!cnt)
		return 0;

	if (cnt >= MSM_OIS_MAX_VREGS) {
		pr_err("%s failed %d cnt %d\n", __func__, __LINE__, cnt);
		return -EINVAL;
	}

	for (i = 0; i < cnt; i++) {
		rc = msm_camera_config_single_vreg(&(o_ctrl->pdev->dev),
			&vreg_cfg->cam_vreg[i],
			(struct regulator **)&vreg_cfg->data[i],
			config);
	}
	return rc;
}

static int32_t msm_ois_power_down(struct msm_ois_ctrl_t *o_ctrl)
{
	int32_t rc = 0;
	CDBG("Enter\n");
	if (o_ctrl->ois_state != OIS_DISABLE_STATE) {

		rc = msm_ois_vreg_control(o_ctrl, 0);
		if (rc < 0) {
			pr_err("%s failed %d\n", __func__, __LINE__);
			return rc;
		}

		o_ctrl->i2c_tbl_index = 0;
		o_ctrl->ois_state = OIS_OPS_INACTIVE;
	}
	CDBG("Exit\n");
	return rc;
}

static int msm_ois_init(struct msm_ois_ctrl_t *o_ctrl)
{
	int rc = 0;
	CDBG("Enter\n");

	if (!o_ctrl) {
		pr_err("failed\n");
		return -EINVAL;
	}

	if (o_ctrl->ois_device_type == MSM_CAMERA_PLATFORM_DEVICE) {
		rc = o_ctrl->i2c_client.i2c_func_tbl->i2c_util(
			&o_ctrl->i2c_client, MSM_CCI_INIT);
		if (rc < 0)
			pr_err("cci_init failed\n");
	}
	o_ctrl->ois_state = OIS_OPS_ACTIVE;
	CDBG("Exit\n");
	return rc;
}

static int32_t msm_ois_control(struct msm_ois_ctrl_t *o_ctrl,
	struct msm_ois_set_info_t *set_info)
{
	struct reg_settings_ois_t *settings = NULL;
	int32_t rc = 0;
	struct msm_camera_cci_client *cci_client = NULL;
	CDBG("Enter\n");

	if (o_ctrl->ois_device_type == MSM_CAMERA_PLATFORM_DEVICE) {
		cci_client = o_ctrl->i2c_client.cci_client;
		cci_client->sid =
			set_info->ois_params.i2c_addr >> 1;
		cci_client->retries = 3;
		cci_client->id_map = 0;
		cci_client->cci_i2c_master = o_ctrl->cci_master;
	} else {
		o_ctrl->i2c_client.client->addr =
			set_info->ois_params.i2c_addr;
	}
	o_ctrl->i2c_client.addr_type = MSM_CAMERA_I2C_WORD_ADDR;


	if (set_info->ois_params.setting_size > 0 &&
		set_info->ois_params.setting_size
		< MAX_OIS_REG_SETTINGS) {
		settings = kmalloc(
			sizeof(struct reg_settings_ois_t) *
			(set_info->ois_params.setting_size),
			GFP_KERNEL);
		if (settings == NULL) {
			pr_err("Error allocating memory\n");
			return -EFAULT;
		}
		if (copy_from_user(settings,
			(void *)set_info->ois_params.settings,
			set_info->ois_params.setting_size *
			sizeof(struct reg_settings_ois_t))) {
			kfree(settings);
			pr_err("Error copying\n");
			return -EFAULT;
		}

		rc = msm_ois_write_settings(o_ctrl,
			set_info->ois_params.setting_size,
			settings);
		kfree(settings);
		if (rc < 0) {
			pr_err("Error\n");
			return -EFAULT;
		}
	}

	CDBG("Exit\n");

	return rc;
}


static int32_t msm_ois_config(struct msm_ois_ctrl_t *o_ctrl,
	void __user *argp)
{
	struct msm_ois_cfg_data *cdata =
		(struct msm_ois_cfg_data *)argp;
	int32_t rc = 0;
	mutex_lock(o_ctrl->ois_mutex);
	CDBG("Enter\n");
	CDBG("%s type %d\n", __func__, cdata->cfgtype);
	switch (cdata->cfgtype) {
	case CFG_OIS_INIT:
		rc = msm_ois_init(o_ctrl);
		if (rc < 0)
			pr_err("msm_ois_init failed %d\n", rc);
		break;
	case CFG_OIS_POWERDOWN:
		rc = msm_ois_power_down(o_ctrl);
		if (rc < 0)
			pr_err("msm_ois_power_down failed %d\n", rc);
		break;
	case CFG_OIS_POWERUP:
		rc = msm_ois_power_up(o_ctrl);
		if (rc < 0)
			pr_err("Failed ois power up%d\n", rc);
		break;
	case CFG_OIS_CONTROL:
		rc = msm_ois_control(o_ctrl, &cdata->cfg.set_info);
		if (rc < 0)
			pr_err("Failed ois control%d\n", rc);
		break;
	case CFG_OIS_I2C_WRITE_SEQ_TABLE: {
		struct msm_camera_i2c_seq_reg_setting conf_array;
		struct msm_camera_i2c_seq_reg_array *reg_setting = NULL;

#ifdef CONFIG_COMPAT
		if (is_compat_task()) {
			memcpy(&conf_array,
				(void *)cdata->cfg.settings,
				sizeof(struct msm_camera_i2c_seq_reg_setting));
		} else
#endif
		if (copy_from_user(&conf_array,
			(void *)cdata->cfg.settings,
			sizeof(struct msm_camera_i2c_seq_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		if (!conf_array.size) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		reg_setting = kzalloc(conf_array.size *
			(sizeof(struct msm_camera_i2c_seq_reg_array)),
			GFP_KERNEL);
		if (!reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(reg_setting, (void *)conf_array.reg_setting,
			conf_array.size *
			sizeof(struct msm_camera_i2c_seq_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(reg_setting);
			rc = -EFAULT;
			break;
		}

		conf_array.reg_setting = reg_setting;
		rc = o_ctrl->i2c_client.i2c_func_tbl->
			i2c_write_seq_table(&o_ctrl->i2c_client,
			&conf_array);
		kfree(reg_setting);
		break;
	}
	default:
		break;
	}
	mutex_unlock(o_ctrl->ois_mutex);
	CDBG("Exit\n");
	return rc;
}

static int32_t msm_ois_get_subdev_id(struct msm_ois_ctrl_t *o_ctrl,
	void *arg)
{
	uint32_t *subdev_id = (uint32_t *)arg;
	CDBG("Enter\n");
	if (!subdev_id) {
		pr_err("failed\n");
		return -EINVAL;
	}
	if (o_ctrl->ois_device_type == MSM_CAMERA_PLATFORM_DEVICE)
		*subdev_id = o_ctrl->pdev->id;
	else
		*subdev_id = o_ctrl->subdev_id;

	CDBG("subdev_id %d\n", *subdev_id);
	CDBG("Exit\n");
	return 0;
}
=======
#include "msm_ois.h"
#include "msm_cci.h"
#include "msm_ois_i2c.h"

#define OIS_MAKER_ID_ADDR		(0x700)

extern void fuji_ois_init(struct msm_ois_ctrl_t *msm_ois_t);
extern void lgit_ois_init(struct msm_ois_ctrl_t *msm_ois_t);
extern void lgit2_ois_init(struct msm_ois_ctrl_t *msm_ois_t);

static int ois_lock = 1;

DEFINE_MSM_MUTEX(msm_ois_mutex);

static struct msm_ois_ctrl_t msm_ois_t;
>>>>>>> 5b5616e... Add LG G3 Support

static struct msm_camera_i2c_fn_t msm_sensor_cci_func_tbl = {
	.i2c_read = msm_camera_cci_i2c_read,
	.i2c_read_seq = msm_camera_cci_i2c_read_seq,
	.i2c_write = msm_camera_cci_i2c_write,
<<<<<<< HEAD
	.i2c_write_table = msm_camera_cci_i2c_write_table,
	.i2c_write_seq = msm_camera_cci_i2c_write_seq,
=======
	.i2c_write_seq = msm_camera_cci_i2c_write_seq,	//sungmin.woo added
	.i2c_write_table = msm_camera_cci_i2c_write_table,
>>>>>>> 5b5616e... Add LG G3 Support
	.i2c_write_seq_table = msm_camera_cci_i2c_write_seq_table,
	.i2c_write_table_w_microdelay =
		msm_camera_cci_i2c_write_table_w_microdelay,
	.i2c_util = msm_sensor_cci_i2c_util,
<<<<<<< HEAD
	.i2c_poll =  msm_camera_cci_i2c_poll,
=======
>>>>>>> 5b5616e... Add LG G3 Support
};

static struct msm_camera_i2c_fn_t msm_sensor_qup_func_tbl = {
	.i2c_read = msm_camera_qup_i2c_read,
	.i2c_read_seq = msm_camera_qup_i2c_read_seq,
	.i2c_write = msm_camera_qup_i2c_write,
<<<<<<< HEAD
	.i2c_write_table = msm_camera_qup_i2c_write_table,
	.i2c_write_seq = msm_camera_qup_i2c_write_seq,
	.i2c_write_seq_table = msm_camera_qup_i2c_write_seq_table,
	.i2c_write_table_w_microdelay =
		msm_camera_qup_i2c_write_table_w_microdelay,
	.i2c_poll = msm_camera_qup_i2c_poll,
};

static int msm_ois_close(struct v4l2_subdev *sd,
	struct v4l2_subdev_fh *fh) {
	int rc = 0;
	struct msm_ois_ctrl_t *o_ctrl =  v4l2_get_subdevdata(sd);
	CDBG("Enter\n");
	if (!o_ctrl || !o_ctrl->i2c_client.i2c_func_tbl) {
		/* check to make sure that init happens before release */
		pr_err("failed\n");
		return -EINVAL;
	}
	mutex_lock(o_ctrl->ois_mutex);
	if (o_ctrl->ois_device_type == MSM_CAMERA_PLATFORM_DEVICE &&
		o_ctrl->ois_state != OIS_DISABLE_STATE) {
		rc = o_ctrl->i2c_client.i2c_func_tbl->i2c_util(
			&o_ctrl->i2c_client, MSM_CCI_RELEASE);
		if (rc < 0)
			pr_err("cci_init failed\n");
	}
	o_ctrl->ois_state = OIS_DISABLE_STATE;
	mutex_unlock(o_ctrl->ois_mutex);
	CDBG("Exit\n");
	return rc;
}

static const struct v4l2_subdev_internal_ops msm_ois_internal_ops = {
	.close = msm_ois_close,
};

static long msm_ois_subdev_ioctl(struct v4l2_subdev *sd,
			unsigned int cmd, void *arg)
{
	int rc;
	struct msm_ois_ctrl_t *o_ctrl = v4l2_get_subdevdata(sd);
	void __user *argp = (void __user *)arg;
	CDBG("Enter\n");
	CDBG("%s:%d o_ctrl %p argp %p\n", __func__, __LINE__, o_ctrl, argp);
	switch (cmd) {
	case VIDIOC_MSM_SENSOR_GET_SUBDEV_ID:
		return msm_ois_get_subdev_id(o_ctrl, argp);
	case VIDIOC_MSM_OIS_CFG:
		return msm_ois_config(o_ctrl, argp);
	case MSM_SD_SHUTDOWN:
		if (!o_ctrl->i2c_client.i2c_func_tbl) {
			pr_err("o_ctrl->i2c_client.i2c_func_tbl NULL\n");
			return -EINVAL;
		} else {
			rc = msm_ois_power_down(o_ctrl);
			if (rc < 0) {
				pr_err("%s:%d OIS Power down failed\n",
					__func__, __LINE__);
			}
			return msm_ois_close(sd, NULL);
		}
	default:
		return -ENOIOCTLCMD;
	}
}

static int32_t msm_ois_power_up(struct msm_ois_ctrl_t *o_ctrl)
{
	int rc = 0;
	CDBG("%s called\n", __func__);

	rc = msm_ois_vreg_control(o_ctrl, 1);
	if (rc < 0) {
		pr_err("%s failed %d\n", __func__, __LINE__);
		return rc;
	}

	o_ctrl->ois_state = OIS_ENABLE_STATE;
	CDBG("Exit\n");
	return rc;
}

static struct v4l2_subdev_core_ops msm_ois_subdev_core_ops = {
	.ioctl = msm_ois_subdev_ioctl,
};

static struct v4l2_subdev_ops msm_ois_subdev_ops = {
	.core = &msm_ois_subdev_core_ops,
};

static const struct i2c_device_id msm_ois_i2c_id[] = {
	{"qcom,ois", (kernel_ulong_t)NULL},
	{ }
};
=======
	.i2c_write_seq = msm_camera_qup_i2c_write_seq,	//sungmin.woo added
	.i2c_write_table = msm_camera_qup_i2c_write_table,
	.i2c_write_seq_table = msm_camera_qup_i2c_write_seq_table,
	.i2c_write_table_w_microdelay =
		msm_camera_qup_i2c_write_table_w_microdelay,
};

static const struct v4l2_subdev_internal_ops msm_ois_internal_ops = {
//	.open = msm_ois_open,
//	.close = msm_ois_close,
};


int32_t ois_i2c_write_table(struct msm_camera_i2c_reg_setting *write_setting)
{
	int32_t ret = 0;
	ret = msm_ois_t.i2c_client.i2c_func_tbl->i2c_write_table(&msm_ois_t.i2c_client, write_setting);
	return ret;	
}
int32_t ois_i2c_write_seq_table(struct msm_camera_i2c_seq_reg_setting *write_setting)
{
	int32_t ret = 0;
	ret= msm_ois_t.i2c_client.i2c_func_tbl->i2c_write_seq_table(&msm_ois_t.i2c_client, write_setting);
	return ret;	
}

int32_t ois_i2c_write(uint16_t addr, uint16_t data, enum msm_camera_i2c_data_type data_type)
{
	int32_t ret = 0;
	ret= msm_ois_t.i2c_client.i2c_func_tbl->i2c_write(&msm_ois_t.i2c_client, addr, data, data_type);
	return ret;
}

int32_t ois_i2c_read(uint16_t addr, uint16_t *data, enum msm_camera_i2c_data_type data_type)
{
	int32_t ret = 0;
	ret= msm_ois_t.i2c_client.i2c_func_tbl->i2c_read(&msm_ois_t.i2c_client, addr, &data[0], data_type);
	return ret;
}

int32_t ois_i2c_write_seq(uint16_t addr, uint8_t *data, uint16_t num_byte)
{
	int32_t ret = 0;
	ret= msm_ois_t.i2c_client.i2c_func_tbl->i2c_write_seq(&msm_ois_t.i2c_client, addr, data, num_byte);
	return ret;
}

int32_t ois_i2c_read_seq(uint16_t addr, uint8_t *data, uint16_t num_byte)
{
	int32_t ret = 0;
	ret= msm_ois_t.i2c_client.i2c_func_tbl->i2c_read_seq(&msm_ois_t.i2c_client, addr, &data[0], num_byte);
	return ret;
}

int32_t ois_i2c_e2p_write(uint16_t addr, uint16_t data, enum msm_camera_i2c_data_type data_type)
{
	int32_t ret = 0;
	struct msm_camera_i2c_client *ois_i2c_client = NULL;	
	ois_i2c_client = &msm_ois_t.i2c_client;
	
	ois_i2c_client->cci_client->sid = 0xA0 >> 1;
	ret = ois_i2c_client->i2c_func_tbl->i2c_write(ois_i2c_client, addr, data, data_type);
	ois_i2c_client->cci_client->sid = msm_ois_t.sid_ois;
	return ret;
}

int32_t ois_i2c_e2p_read(uint16_t addr, uint16_t *data, enum msm_camera_i2c_data_type data_type)
{
	int32_t ret = 0;
	struct msm_camera_i2c_client *ois_i2c_client = NULL;	
	ois_i2c_client = &msm_ois_t.i2c_client;
	
	ois_i2c_client->cci_client->sid = 0xA0 >> 1;
	ret = ois_i2c_client->i2c_func_tbl->i2c_read(ois_i2c_client, addr, data, data_type);
	ois_i2c_client->cci_client->sid = msm_ois_t.sid_ois;
	return ret;
}
int32_t ois_i2c_act_write(uint8_t data1, uint8_t data2)
{
	int32_t ret = 0;
	struct msm_camera_i2c_client *ois_i2c_client = NULL;
	ois_i2c_client = &msm_ois_t.i2c_client;
	
	ois_i2c_client->cci_client->sid = 0x18 >> 1;
	ois_i2c_client->addr_type = MSM_CAMERA_I2C_BYTE_ADDR;

	ret = ois_i2c_client->i2c_func_tbl->i2c_write(ois_i2c_client, data1, data2, 1);

	ois_i2c_client->cci_client->sid = msm_ois_t.sid_ois;
	ois_i2c_client->addr_type = MSM_CAMERA_I2C_WORD_ADDR;
	return ret;
}
>>>>>>> 5b5616e... Add LG G3 Support

static int32_t msm_ois_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;
<<<<<<< HEAD
	struct msm_ois_ctrl_t *ois_ctrl_t = NULL;
	CDBG("Enter\n");

	if (client == NULL) {
		pr_err("msm_ois_i2c_probe: client is null\n");
		return -EINVAL;
	}

	ois_ctrl_t = kzalloc(sizeof(struct msm_ois_ctrl_t),
		GFP_KERNEL);
	if (!ois_ctrl_t) {
		pr_err("%s:%d failed no memory\n", __func__, __LINE__);
		return -ENOMEM;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("i2c_check_functionality failed\n");
		rc = -EINVAL;
		goto probe_failure;
	}

	CDBG("client = 0x%p\n",  client);

	rc = of_property_read_u32(client->dev.of_node, "cell-index",
		&ois_ctrl_t->subdev_id);
	CDBG("cell-index %d, rc %d\n", ois_ctrl_t->subdev_id, rc);
	if (rc < 0) {
		pr_err("failed rc %d\n", rc);
		goto probe_failure;
	}

	ois_ctrl_t->i2c_driver = &msm_ois_i2c_driver;
	ois_ctrl_t->i2c_client.client = client;
	/* Set device type as I2C */
	ois_ctrl_t->ois_device_type = MSM_CAMERA_I2C_DEVICE;
	ois_ctrl_t->i2c_client.i2c_func_tbl = &msm_sensor_qup_func_tbl;
	ois_ctrl_t->ois_v4l2_subdev_ops = &msm_ois_subdev_ops;
	ois_ctrl_t->ois_mutex = &msm_ois_mutex;

	/* Assign name for sub device */
	snprintf(ois_ctrl_t->msm_sd.sd.name, sizeof(ois_ctrl_t->msm_sd.sd.name),
		"%s", ois_ctrl_t->i2c_driver->driver.name);

	/* Initialize sub device */
	v4l2_i2c_subdev_init(&ois_ctrl_t->msm_sd.sd,
		ois_ctrl_t->i2c_client.client,
		ois_ctrl_t->ois_v4l2_subdev_ops);
	v4l2_set_subdevdata(&ois_ctrl_t->msm_sd.sd, ois_ctrl_t);
	ois_ctrl_t->msm_sd.sd.internal_ops = &msm_ois_internal_ops;
	ois_ctrl_t->msm_sd.sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	media_entity_init(&ois_ctrl_t->msm_sd.sd.entity, 0, NULL, 0);
	ois_ctrl_t->msm_sd.sd.entity.type = MEDIA_ENT_T_V4L2_SUBDEV;
	ois_ctrl_t->msm_sd.sd.entity.group_id = MSM_CAMERA_SUBDEV_OIS;
	ois_ctrl_t->msm_sd.close_seq = MSM_SD_CLOSE_2ND_CATEGORY | 0x2;
	msm_sd_register(&ois_ctrl_t->msm_sd);
	ois_ctrl_t->ois_state = OIS_DISABLE_STATE;
	pr_info("msm_ois_i2c_probe: succeeded\n");
	CDBG("Exit\n");

probe_failure:
	kfree(ois_ctrl_t);
	return rc;
}

#ifdef CONFIG_COMPAT
static long msm_ois_subdev_do_ioctl(
	struct file *file, unsigned int cmd, void *arg)
{
	long rc = 0;
	struct video_device *vdev = video_devdata(file);
	struct v4l2_subdev *sd = vdev_to_v4l2_subdev(vdev);
	struct msm_ois_cfg_data32 *u32 =
		(struct msm_ois_cfg_data32 *)arg;
	struct msm_ois_cfg_data ois_data;
	void *parg = arg;
	struct msm_camera_i2c_seq_reg_setting settings;
	struct msm_camera_i2c_seq_reg_setting32 settings32;

	ois_data.cfgtype = u32->cfgtype;

	switch (cmd) {
	case VIDIOC_MSM_OIS_CFG32:
		cmd = VIDIOC_MSM_OIS_CFG;

		switch (u32->cfgtype) {
		case CFG_OIS_CONTROL:
			ois_data.cfg.set_info.ois_params.setting_size =
				u32->cfg.set_info.ois_params.setting_size;
			ois_data.cfg.set_info.ois_params.i2c_addr =
				u32->cfg.set_info.ois_params.i2c_addr;
			ois_data.cfg.set_info.ois_params.i2c_addr_type =
				u32->cfg.set_info.ois_params.i2c_addr_type;
			ois_data.cfg.set_info.ois_params.i2c_data_type =
				u32->cfg.set_info.ois_params.i2c_data_type;
			ois_data.cfg.set_info.ois_params.settings =
				compat_ptr(u32->cfg.set_info.ois_params.
				settings);
			parg = &ois_data;
			break;
		case CFG_OIS_I2C_WRITE_SEQ_TABLE:
			if (copy_from_user(&settings32,
				(void *)compat_ptr(u32->cfg.settings),
				sizeof(
				struct msm_camera_i2c_seq_reg_setting32))) {
				pr_err("copy_from_user failed\n");
				return -EFAULT;
			}

			settings.addr_type = settings32.addr_type;
			settings.delay = settings32.delay;
			settings.size = settings32.size;
			settings.reg_setting =
				compat_ptr(settings32.reg_setting);

			ois_data.cfgtype = u32->cfgtype;
			ois_data.cfg.settings = &settings;
			parg = &ois_data;
			break;
		default:
			parg = &ois_data;
			break;
		}
	}
	rc = msm_ois_subdev_ioctl(sd, cmd, parg);

	return rc;
}

static long msm_ois_subdev_fops_ioctl(struct file *file, unsigned int cmd,
	unsigned long arg)
{
	return video_usercopy(file, cmd, arg, msm_ois_subdev_do_ioctl);
}
#endif

=======
	struct msm_ois_ctrl_t *act_ctrl_t = NULL;
	pr_err("Enter\n");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("i2c_check_functionality failed\n");
		goto probe_failure;
	}

	act_ctrl_t = (struct msm_ois_ctrl_t *)(id->driver_data);
	CDBG("client = %x\n", (unsigned int) client);
	act_ctrl_t->i2c_client.client = client;
	/* Set device type as I2C */
	act_ctrl_t->act_device_type = MSM_CAMERA_I2C_DEVICE;
	act_ctrl_t->i2c_client.i2c_func_tbl = &msm_sensor_qup_func_tbl;

	/* Assign name for sub device */
	snprintf(act_ctrl_t->msm_sd.sd.name, sizeof(act_ctrl_t->msm_sd.sd.name),
		"%s", act_ctrl_t->i2c_driver->driver.name);

	/* Initialize sub device */
	v4l2_i2c_subdev_init(&act_ctrl_t->msm_sd.sd,
		act_ctrl_t->i2c_client.client,
		act_ctrl_t->act_v4l2_subdev_ops);
	v4l2_set_subdevdata(&act_ctrl_t->msm_sd.sd, act_ctrl_t);
	act_ctrl_t->msm_sd.sd.internal_ops = &msm_ois_internal_ops;
	act_ctrl_t->msm_sd.sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	media_entity_init(&act_ctrl_t->msm_sd.sd.entity, 0, NULL, 0);
	act_ctrl_t->msm_sd.sd.entity.type = MEDIA_ENT_T_V4L2_SUBDEV;
	act_ctrl_t->msm_sd.sd.entity.group_id = MSM_CAMERA_SUBDEV_OIS;
	msm_sd_register(&act_ctrl_t->msm_sd);
	CDBG("succeeded\n");
	pr_err("Exit\n");

probe_failure:
	return rc;
}
>>>>>>> 5b5616e... Add LG G3 Support
static int32_t msm_ois_platform_probe(struct platform_device *pdev)
{
	int32_t rc = 0;
	struct msm_camera_cci_client *cci_client = NULL;
<<<<<<< HEAD
	struct msm_ois_ctrl_t *msm_ois_t = NULL;
	struct msm_ois_vreg *vreg_cfg;
	CDBG("Enter\n");
=======
	pr_err("Enter\n");
>>>>>>> 5b5616e... Add LG G3 Support

	if (!pdev->dev.of_node) {
		pr_err("of_node NULL\n");
		return -EINVAL;
	}

<<<<<<< HEAD
	msm_ois_t = kzalloc(sizeof(struct msm_ois_ctrl_t),
		GFP_KERNEL);
	if (!msm_ois_t) {
		pr_err("%s:%d failed no memory\n", __func__, __LINE__);
		return -ENOMEM;
	}
=======
>>>>>>> 5b5616e... Add LG G3 Support
	rc = of_property_read_u32((&pdev->dev)->of_node, "cell-index",
		&pdev->id);
	CDBG("cell-index %d, rc %d\n", pdev->id, rc);
	if (rc < 0) {
<<<<<<< HEAD
		kfree(msm_ois_t);
=======
>>>>>>> 5b5616e... Add LG G3 Support
		pr_err("failed rc %d\n", rc);
		return rc;
	}

	rc = of_property_read_u32((&pdev->dev)->of_node, "qcom,cci-master",
<<<<<<< HEAD
		&msm_ois_t->cci_master);
	CDBG("qcom,cci-master %d, rc %d\n", msm_ois_t->cci_master, rc);
	if (rc < 0 || msm_ois_t->cci_master >= MASTER_MAX) {
		kfree(msm_ois_t);
=======
		&msm_ois_t.cci_master);
	CDBG("qcom,cci-master %d, rc %d\n", msm_ois_t.cci_master, rc);
	if (rc < 0) {
>>>>>>> 5b5616e... Add LG G3 Support
		pr_err("failed rc %d\n", rc);
		return rc;
	}

<<<<<<< HEAD
	if (of_find_property((&pdev->dev)->of_node,
			"qcom,cam-vreg-name", NULL)) {
		vreg_cfg = &msm_ois_t->vreg_cfg;
		rc = msm_camera_get_dt_vreg_data((&pdev->dev)->of_node,
			&vreg_cfg->cam_vreg, &vreg_cfg->num_vreg);
		if (rc < 0) {
			kfree(msm_ois_t);
			pr_err("failed rc %d\n", rc);
			return rc;
		}
	}

	msm_ois_t->ois_v4l2_subdev_ops = &msm_ois_subdev_ops;
	msm_ois_t->ois_mutex = &msm_ois_mutex;

	/* Set platform device handle */
	msm_ois_t->pdev = pdev;
	/* Set device type as platform device */
	msm_ois_t->ois_device_type = MSM_CAMERA_PLATFORM_DEVICE;
	msm_ois_t->i2c_client.i2c_func_tbl = &msm_sensor_cci_func_tbl;
	msm_ois_t->i2c_client.cci_client = kzalloc(sizeof(
		struct msm_camera_cci_client), GFP_KERNEL);
	if (!msm_ois_t->i2c_client.cci_client) {
		kfree(msm_ois_t->vreg_cfg.cam_vreg);
		kfree(msm_ois_t);
=======
	/* Set platform device handle */
	msm_ois_t.pdev = pdev;
	/* Set device type as platform device */
	msm_ois_t.act_device_type = MSM_CAMERA_PLATFORM_DEVICE;
	msm_ois_t.i2c_client.i2c_func_tbl = &msm_sensor_cci_func_tbl;
	msm_ois_t.i2c_client.cci_client = kzalloc(sizeof(
		struct msm_camera_cci_client), GFP_KERNEL);
	if (!msm_ois_t.i2c_client.cci_client) {
>>>>>>> 5b5616e... Add LG G3 Support
		pr_err("failed no memory\n");
		return -ENOMEM;
	}

<<<<<<< HEAD
	cci_client = msm_ois_t->i2c_client.cci_client;
	cci_client->cci_subdev = msm_cci_get_subdev();
	cci_client->cci_i2c_master = msm_ois_t->cci_master;
	v4l2_subdev_init(&msm_ois_t->msm_sd.sd,
		msm_ois_t->ois_v4l2_subdev_ops);
	v4l2_set_subdevdata(&msm_ois_t->msm_sd.sd, msm_ois_t);
	msm_ois_t->msm_sd.sd.internal_ops = &msm_ois_internal_ops;
	msm_ois_t->msm_sd.sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	snprintf(msm_ois_t->msm_sd.sd.name,
		ARRAY_SIZE(msm_ois_t->msm_sd.sd.name), "msm_ois");
	media_entity_init(&msm_ois_t->msm_sd.sd.entity, 0, NULL, 0);
	msm_ois_t->msm_sd.sd.entity.type = MEDIA_ENT_T_V4L2_SUBDEV;
	msm_ois_t->msm_sd.sd.entity.group_id = MSM_CAMERA_SUBDEV_OIS;
	msm_ois_t->msm_sd.close_seq = MSM_SD_CLOSE_2ND_CATEGORY | 0x2;
	msm_sd_register(&msm_ois_t->msm_sd);
	msm_ois_t->ois_state = OIS_DISABLE_STATE;
	msm_ois_v4l2_subdev_fops = v4l2_subdev_fops;
#ifdef CONFIG_COMPAT
	msm_ois_v4l2_subdev_fops.compat_ioctl32 =
		msm_ois_subdev_fops_ioctl;
#endif
	msm_ois_t->msm_sd.sd.devnode->fops =
		&msm_ois_v4l2_subdev_fops;

=======
	/* ois initial settings */
	msm_ois_t.i2c_client.cci_client->sid = 0x7C >> 1; //0x48 >> 1;
	msm_ois_t.i2c_client.cci_client->retries = 3;
	msm_ois_t.i2c_client.cci_client->id_map = 0;
	msm_ois_t.i2c_client.cci_client->cci_i2c_master = MASTER_0;
	/* Update sensor address type */
	msm_ois_t.i2c_client.addr_type = MSM_CAMERA_I2C_WORD_ADDR;

	cci_client = msm_ois_t.i2c_client.cci_client;
	cci_client->cci_subdev = msm_cci_get_subdev();
	v4l2_subdev_init(&msm_ois_t.msm_sd.sd,
		msm_ois_t.act_v4l2_subdev_ops);
	v4l2_set_subdevdata(&msm_ois_t.msm_sd.sd, &msm_ois_t);
	msm_ois_t.msm_sd.sd.internal_ops = &msm_ois_internal_ops;
	msm_ois_t.msm_sd.sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	snprintf(msm_ois_t.msm_sd.sd.name,
		ARRAY_SIZE(msm_ois_t.msm_sd.sd.name), "msm_ois");
	media_entity_init(&msm_ois_t.msm_sd.sd.entity, 0, NULL, 0);
	msm_ois_t.msm_sd.sd.entity.type = MEDIA_ENT_T_V4L2_SUBDEV;
	msm_ois_t.msm_sd.sd.entity.group_id = MSM_CAMERA_SUBDEV_OIS;
	msm_sd_register(&msm_ois_t.msm_sd);

	msm_ois_t.sid_ois = msm_ois_t.i2c_client.cci_client->sid;
	msm_ois_t.ois_func_tbl = NULL;
	
>>>>>>> 5b5616e... Add LG G3 Support
	CDBG("Exit\n");
	return rc;
}

<<<<<<< HEAD
static const struct of_device_id msm_ois_i2c_dt_match[] = {
	{.compatible = "qcom,ois"},
	{}
};

MODULE_DEVICE_TABLE(of, msm_ois_i2c_dt_match);
=======

static const struct i2c_device_id msm_ois_i2c_id[] = {
	{"msm_ois", (kernel_ulong_t)&msm_ois_t},
	{ }
};
>>>>>>> 5b5616e... Add LG G3 Support

static struct i2c_driver msm_ois_i2c_driver = {
	.id_table = msm_ois_i2c_id,
	.probe  = msm_ois_i2c_probe,
	.remove = __exit_p(msm_ois_i2c_remove),
	.driver = {
<<<<<<< HEAD
		.name = "qcom,ois",
		.owner = THIS_MODULE,
		.of_match_table = msm_ois_i2c_dt_match,
=======
		.name = "msm_ois",
>>>>>>> 5b5616e... Add LG G3 Support
	},
};

static const struct of_device_id msm_ois_dt_match[] = {
<<<<<<< HEAD
	{.compatible = "qcom,ois", .data = NULL},
=======
	{.compatible = "qcom,ois"},
>>>>>>> 5b5616e... Add LG G3 Support
	{}
};

MODULE_DEVICE_TABLE(of, msm_ois_dt_match);

static struct platform_driver msm_ois_platform_driver = {
<<<<<<< HEAD
	.probe = msm_ois_platform_probe,
=======
>>>>>>> 5b5616e... Add LG G3 Support
	.driver = {
		.name = "qcom,ois",
		.owner = THIS_MODULE,
		.of_match_table = msm_ois_dt_match,
	},
};

static int __init msm_ois_init_module(void)
{
	int32_t rc = 0;
	CDBG("Enter\n");
<<<<<<< HEAD
	rc = platform_driver_register(&msm_ois_platform_driver);
	if (!rc)
		return rc;
	CDBG("%s:%d rc %d\n", __func__, __LINE__, rc);
	return i2c_add_driver(&msm_ois_i2c_driver);
}

static void __exit msm_ois_exit_module(void)
{
	platform_driver_unregister(&msm_ois_platform_driver);
	i2c_del_driver(&msm_ois_i2c_driver);
	return;
}

module_init(msm_ois_init_module);
module_exit(msm_ois_exit_module);
=======
	rc = platform_driver_probe(msm_ois_t.pdriver,
		msm_ois_platform_probe);
	if (!rc)
		return rc;
	CDBG("%s:%d rc %d\n", __func__, __LINE__, rc);
	return i2c_add_driver(msm_ois_t.i2c_driver);
}

int msm_init_ois(enum ois_ver_t ver)
{
	int rc = 0;
	uint16_t chipid = 0;
	
	ois_i2c_e2p_read(OIS_MAKER_ID_ADDR, &chipid, 1);

	CDBG("Enter %d\n", chipid);


	switch(chipid)
	{
	case 0x01:
		lgit_ois_init(&msm_ois_t);
		msm_ois_t.i2c_client.cci_client->sid = msm_ois_t.sid_ois;
		rc = ois_i2c_read(0x027F, &chipid, 1);
		if (rc < 0) {
				msm_ois_t.ois_func_tbl = NULL;
				printk("%s: kernel ois not supported, rc = %d\n", __func__, rc);
				return OIS_INIT_NOT_SUPPORTED;
		}	
		printk("%s : LGIT OIS module type #1!\n", __func__);
		break;
	case 0x02:
	case 0x05:
		lgit2_ois_init(&msm_ois_t); 
		printk("%s : LGIT OIS module type #2!\n", __func__); 
		break;
	case 0x03:
		fuji_ois_init(&msm_ois_t);
		printk("%s : FujiFilm OIS module!\n", __func__);
		break;
	default:
		printk("%s : unknown module! maker id = %d\n", __func__, chipid);
		msm_ois_t.ois_func_tbl = NULL;
		return OIS_INIT_NOT_SUPPORTED;
	}
	msm_ois_t.i2c_client.cci_client->sid = msm_ois_t.sid_ois;

	if (ois_lock)
	{
		mutex_lock(msm_ois_t.ois_mutex);
		if (msm_ois_t.ois_func_tbl)
		{
			rc = msm_ois_t.ois_func_tbl->ois_on(ver);
			if (rc < 0) {
				msm_ois_t.ois_func_tbl = NULL;
				printk("%s: ois open fail\n", __func__);
			}
		}
		else
		{
			printk("%s: No OIS support!\n",__func__);
		}
		mutex_unlock(msm_ois_t.ois_mutex);
	}
	return rc;
}

int msm_ois_off(void)
{
	int rc = 0;

	CDBG("Enter\n");

	if (ois_lock)
	{
		mutex_lock(msm_ois_t.ois_mutex);
		if (msm_ois_t.ois_func_tbl)
		{
			msm_ois_t.ois_func_tbl->ois_off();
		}
		else
		{
			printk("%s: No OIS support!\n",__func__);
		}
		mutex_unlock(msm_ois_t.ois_mutex);	
	}
	else
	{
		ois_lock = 1;
	}
	return rc;
}

int msm_ois_info(struct msm_sensor_ois_info_t *ois_info)
{
	int32_t rc = 0;
	memset(ois_info, 0, sizeof(struct msm_sensor_ois_info_t));
	
	if (msm_ois_t.ois_func_tbl)
	{
		rc = msm_ois_t.ois_func_tbl->ois_stat(ois_info);
	}
	return rc;
}

int msm_ois_mode(enum ois_mode_t data)
{
	int32_t rc = 0;
	CDBG("%s: mode = %d\n",__func__, data);
	if (msm_ois_t.ois_func_tbl)
	{
		rc = msm_ois_t.ois_func_tbl->ois_mode(data);
	}
	return rc;
}

int msm_ois_move_lens (int16_t target_x, int16_t target_y)
{
	int32_t rc = 0;
	CDBG("%s: target = %d, %d\n",__func__, target_x, target_y);
	if (msm_ois_t.ois_func_tbl)
	{
		rc = msm_ois_t.ois_func_tbl->ois_move_lens(target_x, target_y);
	}
	return rc;
}


static long msm_ois_subdev_ioctl(struct v4l2_subdev *sd,
			unsigned int cmd, void *arg)
{
	struct msm_ois_ctrl_t *a_ctrl = v4l2_get_subdevdata(sd);
	void __user *argp = (void __user *)arg;
	CDBG("Enter\n");
	CDBG("%s:%d a_ctrl %p argp %p\n", __func__, __LINE__, a_ctrl, argp);   
	return -ENOIOCTLCMD;
}

static int32_t msm_ois_power(struct v4l2_subdev *sd, int on)
{
	int rc = 0;
	CDBG("Enter\n");
	CDBG("Exit\n");
	return rc;
}

static struct v4l2_subdev_core_ops msm_ois_subdev_core_ops = {
	.ioctl = msm_ois_subdev_ioctl,
	.s_power = msm_ois_power,
};

static struct v4l2_subdev_ops msm_ois_subdev_ops = {
	.core = &msm_ois_subdev_core_ops,
};

static struct msm_ois_ctrl_t msm_ois_t = {
	.i2c_driver = &msm_ois_i2c_driver,
	.pdriver = &msm_ois_platform_driver,
	.act_v4l2_subdev_ops = &msm_ois_subdev_ops,
	.ois_mutex = &msm_ois_mutex,
};

module_init(msm_ois_init_module);
>>>>>>> 5b5616e... Add LG G3 Support
MODULE_DESCRIPTION("MSM OIS");
MODULE_LICENSE("GPL v2");
