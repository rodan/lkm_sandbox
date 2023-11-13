/*
 * Honeywell TruStability HSC Series pressure/temperature sensor
 *
 * Copyright (c) 2023 Petre Rodan <2b4eda@subdimension.ro>
 *
 * (7-bit I2C slave address can be 0x28, 0x38, 0x48, 0x58,
                                        0x68, 0x78, 0x88 or 0x98)
 *
 * Datasheet: https://prod-edam.honeywell.com/content/dam/honeywell-edam/sps/siot/en-us/products/sensors/pressure-sensors/board-mount-pressure-sensors/trustability-hsc-series/documents/sps-siot-trustability-hsc-series-high-accuracy-board-mount-pressure-sensors-50099148-a-en-ciid-151133.pdf?download=false
 * i2c-related datasheet: https://prod-edam.honeywell.com/content/dam/honeywell-edam/sps/siot/en-us/products/sensors/pressure-sensors/board-mount-pressure-sensors/common/documents/sps-siot-i2c-comms-digital-output-pressure-sensors-tn-008201-3-en-ciid-45841.pdf?download=false
 */

#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/iio/iio.h>
//#include <linux/iio/sysfs.h>

#include "honeywell_hsc.h"

static int hsc_i2c_xfer(struct hsc_data *data)
{
	//const struct hsc_chip_data *chip = data->chip;
	struct i2c_client *client = data->client;
	struct i2c_msg msg;
	int ret;

	msg.addr = client->addr;
	msg.flags = client->flags | I2C_M_RD;
	//msg.len = chip->read_size;
	msg.len = HSC_REG_MEASUREMENT_RD_SIZE;
	msg.buf = (char *)&data->buffer;

	ret = i2c_transfer(client->adapter, &msg, 1);

	return (ret == 2) ? 0 : ret;
}

static int hsc_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct iio_dev *indio_dev;
	struct hsc_data *hsc;
	int chip_id;

	indio_dev = devm_iio_device_alloc(dev, sizeof(*hsc));
	if (!indio_dev) {
		dev_err(&client->dev, "Failed to allocate device\n");
		return -ENOMEM;
	}

	hsc = iio_priv(indio_dev);

	if (i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		hsc->xfer = hsc_i2c_xfer;
	} else {
		return -EOPNOTSUPP;
	}

	dev_fwnode(dev);
	chip_id = id->driver_data;

	pr_info("hsc id 0x%02x found\n", chip_id);
	i2c_set_clientdata(client, indio_dev);
	hsc->client = client;

	return hsc_probe(indio_dev, &client->dev, id->name, id->driver_data);
}

static const struct of_device_id hsc_i2c_match[] = {
	{ .compatible = "honeywell,hsc001ba",}, { .compatible = "honeywell,hsc1.6ba",},
	{ .compatible = "honeywell,hsc2.5ba",}, { .compatible = "honeywell,hsc004ba",},
	{ .compatible = "honeywell,hsc006ba",}, { .compatible = "honeywell,hsc010ba",},
	{ .compatible = "honeywell,hsc1.6md",}, { .compatible = "honeywell,hsc2.5md",},
	{ .compatible = "honeywell,hsc004md",}, { .compatible = "honeywell,hsc006md",},
	{ .compatible = "honeywell,hsc010md",}, { .compatible = "honeywell,hsc016md",},
	{ .compatible = "honeywell,hsc025md",}, { .compatible = "honeywell,hsc040md",},
	{ .compatible = "honeywell,hsc060md",}, { .compatible = "honeywell,hsc100md",},
	{ .compatible = "honeywell,hsc160md",}, { .compatible = "honeywell,hsc250md",},
	{ .compatible = "honeywell,hsc400md",}, { .compatible = "honeywell,hsc600md",},
	{ .compatible = "honeywell,hsc001bd",}, { .compatible = "honeywell,hsc1.6bd",},
	{ .compatible = "honeywell,hsc2.5bd",}, { .compatible = "honeywell,hsc004bd",},
	{ .compatible = "honeywell,hsc2.5mg",}, { .compatible = "honeywell,hsc004mg",},
	{ .compatible = "honeywell,hsc006mg",}, { .compatible = "honeywell,hsc010mg",},
	{ .compatible = "honeywell,hsc016mg",}, { .compatible = "honeywell,hsc025mg",},
	{ .compatible = "honeywell,hsc040mg",}, { .compatible = "honeywell,hsc060mg",},
	{ .compatible = "honeywell,hsc100mg",}, { .compatible = "honeywell,hsc160mg",},
	{ .compatible = "honeywell,hsc250mg",}, { .compatible = "honeywell,hsc400mg",},
	{ .compatible = "honeywell,hsc600mg",}, { .compatible = "honeywell,hsc001bg",},
	{ .compatible = "honeywell,hsc1.6bg",}, { .compatible = "honeywell,hsc2.5bg",},
	{ .compatible = "honeywell,hsc004bg",}, { .compatible = "honeywell,hsc006bg",},
	{ .compatible = "honeywell,hsc010bg",}, { .compatible = "honeywell,hsc100ka",},
	{ .compatible = "honeywell,hsc160ka",}, { .compatible = "honeywell,hsc250ka",},
	{ .compatible = "honeywell,hsc400ka",}, { .compatible = "honeywell,hsc600ka",},
	{ .compatible = "honeywell,hsc001ga",}, { .compatible = "honeywell,hsc160ld",},
	{ .compatible = "honeywell,hsc250ld",}, { .compatible = "honeywell,hsc400ld",},
	{ .compatible = "honeywell,hsc600ld",}, { .compatible = "honeywell,hsc001kd",},
	{ .compatible = "honeywell,hsc1.6kd",}, { .compatible = "honeywell,hsc2.5kd",},
	{ .compatible = "honeywell,hsc004kd",}, { .compatible = "honeywell,hsc006kd",},
	{ .compatible = "honeywell,hsc010kd",}, { .compatible = "honeywell,hsc016kd",},
	{ .compatible = "honeywell,hsc025kd",}, { .compatible = "honeywell,hsc040kd",},
	{ .compatible = "honeywell,hsc060kd",}, { .compatible = "honeywell,hsc100kd",},
	{ .compatible = "honeywell,hsc160kd",}, { .compatible = "honeywell,hsc250kd",},
	{ .compatible = "honeywell,hsc400kd",}, { .compatible = "honeywell,hsc250lg",},
	{ .compatible = "honeywell,hsc400lg",}, { .compatible = "honeywell,hsc600lg",},
	{ .compatible = "honeywell,hsc001kg",}, { .compatible = "honeywell,hsc1.6kg",},
	{ .compatible = "honeywell,hsc2.5kg",}, { .compatible = "honeywell,hsc004kg",},
	{ .compatible = "honeywell,hsc006kg",}, { .compatible = "honeywell,hsc010kg",},
	{ .compatible = "honeywell,hsc016kg",}, { .compatible = "honeywell,hsc025kg",},
	{ .compatible = "honeywell,hsc040kg",}, { .compatible = "honeywell,hsc060kg",},
	{ .compatible = "honeywell,hsc100kg",}, { .compatible = "honeywell,hsc160kg",},
	{ .compatible = "honeywell,hsc250kg",}, { .compatible = "honeywell,hsc400kg",},
	{ .compatible = "honeywell,hsc600kg",}, { .compatible = "honeywell,hsc001gg",},
	{ .compatible = "honeywell,hsc015pa",}, { .compatible = "honeywell,hsc030pa",},
	{ .compatible = "honeywell,hsc060pa",}, { .compatible = "honeywell,hsc100pa",},
	{ .compatible = "honeywell,hsc150pa",}, { .compatible = "honeywell,hsc0.5nd",},
	{ .compatible = "honeywell,hsc001nd",}, { .compatible = "honeywell,hsc002nd",},
	{ .compatible = "honeywell,hsc004nd",}, { .compatible = "honeywell,hsc005nd",},
	{ .compatible = "honeywell,hsc010nd",}, { .compatible = "honeywell,hsc020nd",},
	{ .compatible = "honeywell,hsc030nd",}, { .compatible = "honeywell,hsc001pd",},
	{ .compatible = "honeywell,hsc005pd",}, { .compatible = "honeywell,hsc015pd",},
	{ .compatible = "honeywell,hsc030pd",}, { .compatible = "honeywell,hsc060pd",},
	{ .compatible = "honeywell,hsc001ng",}, { .compatible = "honeywell,hsc002ng",},
	{ .compatible = "honeywell,hsc004ng",}, { .compatible = "honeywell,hsc005ng",},
	{ .compatible = "honeywell,hsc010ng",}, { .compatible = "honeywell,hsc020ng",},
	{ .compatible = "honeywell,hsc030ng",}, { .compatible = "honeywell,hsc001pg",},
	{ .compatible = "honeywell,hsc005pg",}, { .compatible = "honeywell,hsc015pg",},
	{ .compatible = "honeywell,hsc030pg",}, { .compatible = "honeywell,hsc060pg",},
	{ .compatible = "honeywell,hsc100pg",}, { .compatible = "honeywell,hsc150pg",},
	{},
};

MODULE_DEVICE_TABLE(of, hsc_i2c_match);

static const struct i2c_device_id hsc_id[] = {
	{ "hsc001ba", HSC001BA }, { "hsc1.6ba", HSC1_6BA },
	{ "hsc2.5ba", HSC2_5BA }, { "hsc004ba", HSC004BA },
	{ "hsc006ba", HSC006BA }, { "hsc010ba", HSC010BA },
	{ "hsc1.6md", HSC1_6MD }, { "hsc2.5md", HSC2_5MD },
	{ "hsc004md", HSC004MD }, { "hsc006md", HSC006MD },
	{ "hsc010md", HSC010MD }, { "hsc016md", HSC016MD },
	{ "hsc025md", HSC025MD }, { "hsc040md", HSC040MD },
	{ "hsc060md", HSC060MD }, { "hsc100md", HSC100MD },
	{ "hsc160md", HSC160MD }, { "hsc250md", HSC250MD },
	{ "hsc400md", HSC400MD }, { "hsc600md", HSC600MD },
	{ "hsc001bd", HSC001BD }, { "hsc1.6bd", HSC1_6BD },
	{ "hsc2.5bd", HSC2_5BD }, { "hsc004bd", HSC004BD },
	{ "hsc2.5mg", HSC2_5MG }, { "hsc004mg", HSC004MG },
	{ "hsc006mg", HSC006MG }, { "hsc010mg", HSC010MG },
	{ "hsc016mg", HSC016MG }, { "hsc025mg", HSC025MG },
	{ "hsc040mg", HSC040MG }, { "hsc060mg", HSC060MG },
	{ "hsc100mg", HSC100MG }, { "hsc160mg", HSC160MG },
	{ "hsc250mg", HSC250MG }, { "hsc400mg", HSC400MG },
	{ "hsc600mg", HSC600MG }, { "hsc001bg", HSC001BG },
	{ "hsc1.6bg", HSC1_6BG }, { "hsc2.5bg", HSC2_5BG },
	{ "hsc004bg", HSC004BG }, { "hsc006bg", HSC006BG },
	{ "hsc010bg", HSC010BG }, { "hsc100ka", HSC100KA },
	{ "hsc160ka", HSC160KA }, { "hsc250ka", HSC250KA },
	{ "hsc400ka", HSC400KA }, { "hsc600ka", HSC600KA },
	{ "hsc001ga", HSC001GA }, { "hsc160ld", HSC160LD },
	{ "hsc250ld", HSC250LD }, { "hsc400ld", HSC400LD },
	{ "hsc600ld", HSC600LD }, { "hsc001kd", HSC001KD },
	{ "hsc1.6kd", HSC1_6KD }, { "hsc2.5kd", HSC2_5KD },
	{ "hsc004kd", HSC004KD }, { "hsc006kd", HSC006KD },
	{ "hsc010kd", HSC010KD }, { "hsc016kd", HSC016KD },
	{ "hsc025kd", HSC025KD }, { "hsc040kd", HSC040KD },
	{ "hsc060kd", HSC060KD }, { "hsc100kd", HSC100KD },
	{ "hsc160kd", HSC160KD }, { "hsc250kd", HSC250KD },
	{ "hsc400kd", HSC400KD }, { "hsc250lg", HSC250LG },
	{ "hsc400lg", HSC400LG }, { "hsc600lg", HSC600LG },
	{ "hsc001kg", HSC001KG }, { "hsc1.6kg", HSC1_6KG },
	{ "hsc2.5kg", HSC2_5KG }, { "hsc004kg", HSC004KG },
	{ "hsc006kg", HSC006KG }, { "hsc010kg", HSC010KG },
	{ "hsc016kg", HSC016KG }, { "hsc025kg", HSC025KG },
	{ "hsc040kg", HSC040KG }, { "hsc060kg", HSC060KG },
	{ "hsc100kg", HSC100KG }, { "hsc160kg", HSC160KG },
	{ "hsc250kg", HSC250KG }, { "hsc400kg", HSC400KG },
	{ "hsc600kg", HSC600KG }, { "hsc001gg", HSC001GG },
	{ "hsc015pa", HSC015PA }, { "hsc030pa", HSC030PA },
	{ "hsc060pa", HSC060PA }, { "hsc100pa", HSC100PA },
	{ "hsc150pa", HSC150PA }, { "hsc0.5nd", HSC0_5ND },
	{ "hsc001nd", HSC001ND }, { "hsc002nd", HSC002ND },
	{ "hsc004nd", HSC004ND }, { "hsc005nd", HSC005ND },
	{ "hsc010nd", HSC010ND }, { "hsc020nd", HSC020ND },
	{ "hsc030nd", HSC030ND }, { "hsc001pd", HSC001PD },
	{ "hsc005pd", HSC005PD }, { "hsc015pd", HSC015PD },
	{ "hsc030pd", HSC030PD }, { "hsc060pd", HSC060PD },
	{ "hsc001ng", HSC001NG }, { "hsc002ng", HSC002NG },
	{ "hsc004ng", HSC004NG }, { "hsc005ng", HSC005NG },
	{ "hsc010ng", HSC010NG }, { "hsc020ng", HSC020NG },
	{ "hsc030ng", HSC030NG }, { "hsc001pg", HSC001PG },
	{ "hsc005pg", HSC005PG }, { "hsc015pg", HSC015PG },
	{ "hsc030pg", HSC030PG }, { "hsc060pg", HSC060PG },
	{ "hsc100pg", HSC100PG }, { "hsc150pg", HSC150PG },
	{}
};
MODULE_DEVICE_TABLE(i2c, hsc_id);

static struct i2c_driver hsc_driver = {
	.driver = {
		   .name = "honeywell_hsc",
		   .of_match_table = hsc_i2c_match,
		   },
	.probe = hsc_i2c_probe,
	.id_table = hsc_id,
};
module_i2c_driver(hsc_driver);

MODULE_AUTHOR("Petre Rodan <2b4eda@subdimension.ro>");
MODULE_DESCRIPTION("Honeywell HSC pressure sensor i2c driver");
MODULE_LICENSE("GPL");
MODULE_IMPORT_NS(IIO_HONEYWELL_HSC);
