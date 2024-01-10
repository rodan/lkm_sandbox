// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2016 - Marcin Malagowski <mrc@bourne.st>
 *
 * Datasheet: https://prod-edam.honeywell.com/content/dam/honeywell-edam/sps/siot/en-us/products/sensors/pressure-sensors/board-mount-pressure-sensors/common/documents/sps-siot-i2c-comms-digital-output-pressure-sensors-tn-008201-3-en-ciid-45841.pdf
 * Datasheet: https://prod-edam.honeywell.com/content/dam/honeywell-edam/sps/siot/en-us/products/sensors/pressure-sensors/common/documents/sps-siot-sleep-mode-technical-note-008286-1-en-ciid-155793.pdf
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/types.h>

#include "abp060mg.h"

static int abp060mg_i2c_recv(struct abp_state *state)
{
	struct i2c_client *client = to_i2c_client(state->dev);
	struct i2c_msg msg;
	__be16 buf[2];
	int ret;

	if (state->func_spec->capabilities & ABP_CAP_SLEEP) {
		/*
		 * Send the Full Measurement Request (FMR) command on the CS
		 * line in order to wake up the sensor as per
		 * "Sleep Mode for Use with Honeywell Digital Pressure Sensors"
		 * technical note (consult the datasheet link in the header).
		 *
		 * These specifications require a dummy packet comprised only by
		 * a single byte that contains the 7bit slave address and the
		 * READ bit followed by a STOP.
		 * Because the i2c API does not allow packets without a payload,
		 * the driver sends two bytes in this implementation and hopes
		 * the sensor will not misbehave.
		 */
		buf[0] = 0;
		ret = i2c_master_recv(client, (u8 *)&buf, state->mreq_len);
		if (ret < 0)
			return ret;
	}

	msleep_interruptible(ABP_RESP_TIME_MS);

	msg.addr = client->addr;
	msg.flags = client->flags | I2C_M_RD;
	msg.len = state->read_len;
	msg.buf = state->buffer;
	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0)
		return ret;

	return 0;
}

static int abp060mg_i2c_probe(struct i2c_client *client)
{
	const struct i2c_device_id *id = i2c_client_get_device_id(client);
	u32 flags = ABP_FLAG_NULL;

	if (!id) {
		return -EOPNOTSUPP;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		return -EOPNOTSUPP;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_QUICK))
		flags |= ABP_FLAG_MREQ;

	return abp060mg_common_probe(&client->dev, abp060mg_i2c_recv,
				     id->driver_data, id->name, flags);
}

static const struct i2c_device_id abp060mg_i2c_id_table[] = {
	/* mbar & kPa variants (abp060m [60 mbar] == abp006k [6 kPa]) */
	/*    gage: */
	{ "abp060mg", ABP006KG }, { "abp006kg", ABP006KG },
	{ "abp100mg", ABP010KG }, { "abp010kg", ABP010KG },
	{ "abp160mg", ABP016KG }, { "abp016kg", ABP016KG },
	{ "abp250mg", ABP025KG }, { "abp025kg", ABP025KG },
	{ "abp400mg", ABP040KG }, { "abp040kg", ABP040KG },
	{ "abp600mg", ABP060KG }, { "abp060kg", ABP060KG },
	{ "abp001bg", ABP100KG }, { "abp100kg", ABP100KG },
	{ "abp1_6bg", ABP160KG }, { "abp160kg", ABP160KG },
	{ "abp2_5bg", ABP250KG }, { "abp250kg", ABP250KG },
	{ "abp004bg", ABP400KG }, { "abp400kg", ABP400KG },
	{ "abp006bg", ABP600KG }, { "abp600kg", ABP600KG },
	{ "abp010bg", ABP001GG }, { "abp001gg", ABP001GG },
	/*    differential: */
	{ "abp060md", ABP006KD }, { "abp006kd", ABP006KD },
	{ "abp100md", ABP010KD }, { "abp010kd", ABP010KD },
	{ "abp160md", ABP016KD }, { "abp016kd", ABP016KD },
	{ "abp250md", ABP025KD }, { "abp025kd", ABP025KD },
	{ "abp400md", ABP040KD }, { "abp040kd", ABP040KD },
	{ "abp600md", ABP060KD }, { "abp060kd", ABP060KD },
	{ "abp001bd", ABP100KD }, { "abp100kd", ABP100KD },
	{ "abp1_6bd", ABP160KD }, { "abp160kd", ABP160KD },
	{ "abp2_5bd", ABP250KD }, { "abp250kd", ABP250KD },
	{ "abp004bd", ABP400KD }, { "abp400kd", ABP400KD },
	/* psi variants */
	/*    gage: */
	{ "abp001pg", ABP001PG },
	{ "abp005pg", ABP005PG },
	{ "abp015pg", ABP015PG },
	{ "abp030pg", ABP030PG },
	{ "abp060pg", ABP060PG },
	{ "abp100pg", ABP100PG },
	{ "abp150pg", ABP150PG },
	/*    differential: */
	{ "abp001pd", ABP001PD },
	{ "abp005pd", ABP005PD },
	{ "abp015pd", ABP015PD },
	{ "abp030pd", ABP030PD },
	{ "abp060pd", ABP060PD },
	{}
};
MODULE_DEVICE_TABLE(i2c, abp060mg_i2c_id_table);

static struct i2c_driver abp060mg_i2c_driver = {
	.driver = {
		.name = "abp060mg",
	},
	.probe = abp060mg_i2c_probe,
	.id_table = abp060mg_i2c_id_table,
};
module_i2c_driver(abp060mg_i2c_driver);

MODULE_AUTHOR("Marcin Malagowski <mrc@bourne.st>");
MODULE_DESCRIPTION("Honeywell ABP pressure sensor i2c driver");
MODULE_LICENSE("GPL");
MODULE_IMPORT_NS(IIO_HONEYWELL_ABP060MG);
