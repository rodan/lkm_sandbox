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
	struct i2c_client *client = to_i2c_client(data->dev);
	struct i2c_msg msg;
	int ret;

	msg.addr = client->addr;
	msg.flags = client->flags | I2C_M_RD;
	msg.len = HSC_REG_MEASUREMENT_RD_SIZE;
	msg.buf = data->buffer;

	ret = i2c_transfer(client->adapter, &msg, 1);

	return (ret == 2) ? 0 : ret;
}

static int mpr_i2c_probe(struct i2c_client *client)
{
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_READ_BYTE))
		return -EOPNOTSUPP;

	return mpr_common_probe(&client->dev, mpr_i2c_xfer);
}

static const struct of_device_id hsc_i2c_match[] = {
	{ .compatible = "honeywell,hsc030pa" },
	{}
};
MODULE_DEVICE_TABLE(of, hsc_i2c_match);

static const struct i2c_device_id hsc_i2c_id[] = {
	{ "hsc030pa" },
	{}
};
MODULE_DEVICE_TABLE(i2c, hsc_i2c_id);

static struct i2c_driver hsc_i2c_driver = {
	.driver = {
		.name = "hsc030pa",
		.of_match_table = hsc_i2c_match,
	},
	.probe = hsc_i2c_probe,
	.id_table = hsc_i2c_id,
};
module_i2c_driver(hsc_i2c_driver);

MODULE_AUTHOR("Petre Rodan <petre.rodan@subdimension.ro>");
MODULE_DESCRIPTION("Honeywell HSC and SSC pressure sensor i2c driver");
MODULE_LICENSE("GPL");
MODULE_IMPORT_NS(IIO_HONEYWELL_HSC030PA);
