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

static int mpr_i2c_xfer(struct mpr_data *data, const u8 cmd, const u8 pkt_len)
{
	struct i2c_client *client = to_i2c_client(data->dev);
	struct i2c_msg msg;

	if (pkt_len > MPR_MEASUREMENT_RD_SIZE)
		return -EOVERFLOW;

	msg.addr = client->addr;
	msg.flags = client->flags | I2C_M_RD;
	msg.len = pkt_len;
	msg.buf = data->buffer;

	memset(data->buffer, 0, MPR_MEASUREMENT_RD_SIZE);
	data->buffer[0] = cmd;

	return i2c_transfer(client->adapter, &msg, 1);
}

static int mpr_i2c_probe(struct i2c_client *client)
{
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_READ_BYTE))
		return -EOPNOTSUPP;

	return mpr_common_probe(&client->dev, mpr_i2c_xfer, client->irq);
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
