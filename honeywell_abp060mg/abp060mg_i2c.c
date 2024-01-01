// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2016 - Marcin Malagowski <mrc@bourne.st>
 * Copyright (C) 2024 - Petre Rodan <petre.rodan@subdimension.ro>
 */

#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>

#include "abp060mg.h"

static int abp060mg_i2c_recv(struct abp_state *state)
{
	struct i2c_client *client = to_i2c_client(state->dev);
	__be16 buf[2];
	int ret;

	buf[0] = 0;
	ret = i2c_master_send(client, (u8 *)&buf, state->mreq_len);
	if (ret < 0)
		return ret;

	msleep_interruptible(ABP_RESP_TIME_MS);

	ret = i2c_master_recv(client, state->buffer, state->read_len);
	if (ret < 0)
		return ret;

	return 0;
}

static int abp060mg_i2c_probe(struct i2c_client *client)
{
	const struct i2c_device_id *id = i2c_client_get_device_id(client);
	u32 flags = ABP_FLAG_NULL;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		return -EOPNOTSUPP;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_QUICK))
		flags |= ABP_FLAG_MREQ;

	return abp060mg_common_probe(&client->dev, abp060mg_i2c_recv,
				     id->driver_data, flags);
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
	{ /* empty */ },
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
