/*
 * Honeywell TruStability HSC Series pressure/temperature sensor
 *
 * Copyright (c) 2024 Petre Rodan <2b4eda@subdimension.ro>
 *
 * (7-bit I2C slave address can be 0x28, 0x38, 0x48, 0x58, 
                                        0x68, 0x78, 0x88 or 0x98)
 *
 * Datasheet: https://prod-edam.honeywell.com/content/dam/honeywell-edam/sps/siot/en-us/products/sensors/pressure-sensors/board-mount-pressure-sensors/trustability-hsc-series/documents/sps-siot-trustability-hsc-series-high-accuracy-board-mount-pressure-sensors-50099148-a-en-ciid-151133.pdf?download=false
 * Website: https://sps.honeywell.com/us/en/products/advanced-sensing-technologies/healthcare-sensing/board-mount-pressure-sensors/trustability-hsc-series#resources
 * i2c datasheet: https://prod-edam.honeywell.com/content/dam/honeywell-edam/sps/siot/en-us/products/sensors/pressure-sensors/board-mount-pressure-sensors/common/documents/sps-siot-i2c-comms-digital-output-pressure-sensors-tn-008201-3-en-ciid-45841.pdf?download=false
 */

#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/mod_devicetable.h>
#include <linux/printk.h>

#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>

#define HSC_REG_MEASUREMENT_RD_SIZE 4

struct hsc_chip_data;

struct hsc_data {
    struct i2c_client *client;
    const struct hsc_chip_data *chip;
    struct mutex lock;
    int (*xfer)(struct hsc_data *data);
    bool is_valid;
    unsigned long last_update;
    u8 buffer[4];
};

struct hsc_chip_data {
	bool (*valid)(struct hsc_data *data);
	const struct iio_chan_spec *channels;
	u8 num_channels;
	u8 read_size;
};

static IIO_CONST_ATTR(in_concentration_temp_scale, "0.00000698689");
static IIO_CONST_ATTR(in_concentration_pres_scale, "0.00000000436681223");

static struct attribute *hsc_attributes[] = {
	&iio_const_attr_in_concentration_temp_scale.dev_attr.attr,
	&iio_const_attr_in_concentration_pres_scale.dev_attr.attr,
	NULL,
};

static const struct attribute_group hsc_attrs_group = {
	.attrs = hsc_attributes,
};

static bool hsc_measurement_is_valid(struct hsc_data *data)
{
    return true;
}

static int hsc_i2c_xfer(struct hsc_data *data)
{
    const struct hsc_chip_data *chip = data->chip;
	struct i2c_client *client = data->client;
	struct i2c_msg msg;
	int ret;

	msg.addr = client->addr;
	msg.flags = client->flags | I2C_M_RD;
    msg.len = chip->read_size;
	msg.buf = (char *) &data->buffer;

	ret = i2c_transfer(client->adapter, &msg, 1);

	return (ret == 2) ? 0 : ret;
}

static int hsc_smbus_xfer(struct hsc_data *data)
{
    pr_info("smb xfer not implemented\n");
    return 0;
}

static int hsc_get_measurement(struct hsc_data *data)
{
	int ret;

	/* don't bother sensor more than once a second */
	if (!time_after(jiffies, data->last_update + HZ)) {
		return data->is_valid ? 0 : -EAGAIN;
    }

	data->is_valid = false;
	data->last_update = jiffies;

	ret = data->xfer(data);

    pr_info("recvd %02x %02x %02x %02x\n", data->buffer[0], data->buffer[1], data->buffer[2], data->buffer[3]);
	if (ret < 0)
		return ret;

#if 0
	ret = chip->valid(data);
	if (ret)
		return -EAGAIN;
#endif

	data->is_valid = true;

	return 0;
}

static int hsc_read_raw(struct iio_dev *indio_dev,
			     struct iio_chan_spec const *channel, int *val,
			     int *val2, long mask)
{
	struct hsc_data *data = iio_priv(indio_dev);
	int ret = -EINVAL;

    switch (mask) {

    case IIO_CHAN_INFO_RAW:
        mutex_lock(&data->lock);
        ret = hsc_get_measurement(data);
        mutex_unlock(&data->lock);

        if (ret)
            return ret;

        switch (channel->type) {
		case IIO_PRESSURE:
			*val = ((data->buffer[0] & 0x3f) << 8) + data->buffer[1];
			return IIO_VAL_INT;
		case IIO_TEMP:
            *val = ((data->buffer[2] << 8) + (data->buffer[3] & 0xe0)) >> 5;
            ret = 0;
			if (!ret)
				return IIO_VAL_INT;
			break;
		default:
			return -EINVAL;
		}
		break;

	case IIO_CHAN_INFO_SCALE:
		switch (channel->type) {
		case IIO_PRESSURE:
			*val = 10;
			return IIO_VAL_INT;
		default:
			return -EINVAL;
		}
		break;

	case IIO_CHAN_INFO_OFFSET:
		switch (channel->channel2) {
		case IIO_PRESSURE:
			*val = 44;
			*val2 = 250000;
			return IIO_VAL_INT_PLUS_MICRO;
		case IIO_TEMP:
			*val = -13;
			return IIO_VAL_INT;
		default:
			return -EINVAL;
		}
    }

    return ret;
}

static const struct iio_chan_spec hsc_channels[] = {
	{
		.type = IIO_PRESSURE,
		.info_mask_separate = BIT(IIO_CHAN_INFO_OFFSET) | 
                    BIT(IIO_CHAN_INFO_RAW) | 
                    BIT(IIO_CHAN_INFO_PROCESSED),
	},
	{
		.type = IIO_TEMP,
		.info_mask_separate = BIT(IIO_CHAN_INFO_PROCESSED) |
                    BIT(IIO_CHAN_INFO_RAW) |
                    BIT(IIO_CHAN_INFO_OFFSET),
	},
};

static const struct iio_info hsc_info = {
    .attrs    = &hsc_attrs_group,
	.read_raw = hsc_read_raw,
};

static const struct hsc_chip_data hsc_chips[] = {
	{
		.valid = hsc_measurement_is_valid,
		.read_size = HSC_REG_MEASUREMENT_RD_SIZE,

		.channels = hsc_channels,
		.num_channels = ARRAY_SIZE(hsc_channels),
	},
};


static int hsc_probe(struct i2c_client *client, const struct i2c_device_id *id)
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
	} else if (i2c_check_functionality(client->adapter,
				I2C_FUNC_SMBUS_WORD_DATA | I2C_FUNC_SMBUS_BYTE)) {
		hsc->xfer = hsc_smbus_xfer;
        pr_info("start to panic, smbus selected\n");
    } else {
		return -EOPNOTSUPP;
    }

	if (!dev_fwnode(dev))
		chip_id = id->driver_data;
	else
		chip_id = (unsigned long)device_get_match_data(dev);

    i2c_set_clientdata(client, indio_dev);
    hsc->client = client;
    hsc->chip = &hsc_chips[chip_id];
  	hsc->last_update = jiffies - HZ;

	mutex_init(&hsc->lock);
	indio_dev->name = id->name;
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->info = &hsc_info;
    indio_dev->channels = hsc->chip->channels;
    indio_dev->num_channels = hsc->chip->num_channels;

    return devm_iio_device_register(dev, indio_dev);
}

static void hsc_remove(struct i2c_client *client)
{
#if 0
	struct hsc_data *data = i2c_get_clientdata(client);

	disable_irq(data->irq);
	sysfs_remove_group(&client->dev.kobj, &hsc_attr_group);
	hsc_free_input_device(data);
	hsc_free_object_table(data);
	regulator_bulk_disable(ARRAY_SIZE(data->regulators),
			       data->regulators);
#endif
}

static const struct of_device_id hsc_of_match[] = {
	{ .compatible = "honeywell,hsc", },
	{ .compatible = "honeywell,ssc", },
	{},
};
MODULE_DEVICE_TABLE(of, hsc_of_match);

static const struct i2c_device_id hsc_id[] = {
	{ "honeywell_hsc", 0 },
	{ "honeywell_ssc", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, hsc_id);

static struct i2c_driver hsc_driver = {
	.driver = {
		.name	= "honeywell_hsc",
		.of_match_table = hsc_of_match,
	},
	.probe		= hsc_probe,
	.remove		= hsc_remove,
	.id_table	= hsc_id,
};

module_i2c_driver(hsc_driver);

MODULE_AUTHOR("Petre Rodan <2b4eda@subdimension.ro>");
MODULE_DESCRIPTION("Honeywell HSC SSC pressure sensor driver");
MODULE_LICENSE("GPL");

