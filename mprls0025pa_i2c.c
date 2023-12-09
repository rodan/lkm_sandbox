// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * MPRLS0025PA - Honeywell MicroPressure pressure sensor series driver
 *
 * Copyright (c) Andreas Klinger <ak@it-klinger.de>
 *
 * Data sheet:
 *  https://prod-edam.honeywell.com/content/dam/honeywell-edam/sps/siot/en-us/products/sensors/pressure-sensors/board-mount-pressure-sensors/micropressure-mpr-series/documents/sps-siot-mpr-series-datasheet-32332628-ciid-172626.pdf
 */

#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>

#include <linux/iio/iio.h>

#include "mprls0025pa.h"

static int mpr_i2c_xfer(struct mpr_data *data)
{
#if 0
	struct i2c_client *client = to_i2c_client(data->dev);
	struct i2c_msg msg;
	int ret;

	msg.addr = client->addr;
	msg.flags = client->flags | I2C_M_RD;
	msg.len = HSC_REG_MEASUREMENT_RD_SIZE;
	msg.buf = data->buffer;

	ret = i2c_transfer(client->adapter, &msg, 1);

	return (ret == 2) ? 0 : ret;
#endif
	struct device *dev = data->dev;
	struct i2c_client *client = to_i2c_client(data->dev);
	int ret, i;
	u8 wdata[] = {0xAA, 0x00, 0x00};
	s32 status;
	int nloops = 10;
	//u8 buf[4];

	reinit_completion(&data->completion);

	ret = i2c_master_send(client, wdata, sizeof(wdata));
	if (ret < 0) {
		dev_err(dev, "error while writing ret: %d\n", ret);
		return ret;
	}
	if (ret != sizeof(wdata)) {
		dev_err(dev, "received size doesn't fit - ret: %d / %u\n", ret,
			(u32)sizeof(wdata));
		return -EIO;
	}

	if (data->irq > 0) {
		ret = wait_for_completion_timeout(&data->completion, HZ);
		if (!ret) {
			dev_err(dev, "timeout while waiting for eoc irq\n");
			return -ETIMEDOUT;
		}
	} else {
		/* wait until status indicates data is ready */
		for (i = 0; i < nloops; i++) {
			/*
			 * datasheet only says to wait at least 5 ms for the
			 * data but leave the maximum response time open
			 * --> let's try it nloops (10) times which seems to be
			 *     quite long
			 */
			usleep_range(5000, 10000);
			status = i2c_smbus_read_byte(client);
			if (status < 0) {
				dev_err(dev,
					"error while reading, status: %d\n",
					status);
				return status;
			}
			if (!(status & MPR_I2C_BUSY))
				break;
		}
		if (i == nloops) {
			dev_err(dev, "timeout while reading\n");
			return -ETIMEDOUT;
		}
	}

	ret = i2c_master_recv(client, data->buffer, MPR_MEASUREMENT_RD_SIZE);
	if (ret < 0) {
		dev_err(dev, "error in i2c_master_recv ret: %d\n", ret);
		return ret;
	}
	if (ret != MPR_MEASUREMENT_RD_SIZE) {
		dev_err(dev, "received size doesn't match - ret: %d / %u\n",
			ret, MPR_MEASUREMENT_RD_SIZE);
		return -EIO;
	}

	if (data->buffer[0] & MPR_I2C_BUSY) {
		/*
		 * it should never be the case that status still indicates
		 * business
		 */
		dev_err(dev, "data still not ready: %08x\n", data->buffer[0]);
		return -ETIMEDOUT;
	}

	return 0;
}

static int mpr_i2c_probe(struct i2c_client *client)
{
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_READ_BYTE))
		return -EOPNOTSUPP;

	return mpr_common_probe(&client->dev, mpr_i2c_xfer);
}


static const struct of_device_id mpr_i2c_match[] = {
	{ .compatible = "honeywell,mprls0025pa" },
	{}
};
MODULE_DEVICE_TABLE(of, mpr_i2c_match);

static const struct i2c_device_id mpr_i2c_id[] = {
	{ "mprls0025pa" },
	{}
};
MODULE_DEVICE_TABLE(i2c, mpr_i2c_id);

static struct i2c_driver mpr_i2c_driver = {
	.probe = mpr_i2c_probe,
	.id_table = mpr_i2c_id,
	.driver = {
		.name = "mprls0025pa",
		.of_match_table = mpr_i2c_match,
	},
};
module_i2c_driver(mpr_i2c_driver);

MODULE_AUTHOR("Andreas Klinger <ak@it-klinger.de>");
MODULE_DESCRIPTION("Honeywell MPR pressure sensor i2c driver");
MODULE_LICENSE("GPL");
MODULE_IMPORT_NS(IIO_HONEYWELL_MPRLS0025PA);
