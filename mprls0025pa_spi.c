// SPDX-License-Identifier: GPL-2.0-only
/*
 * MPRLS0025PA - Honeywell MicroPressure MPR series SPI sensor driver
 *
 * Copyright (c) 2024 Petre Rodan <petre.rodan@subdimension.ro>
 *
 * Data sheet:
 *  https://prod-edam.honeywell.com/content/dam/honeywell-edam/sps/siot/en-us/products/sensors/pressure-sensors/board-mount-pressure-sensors/micropressure-mpr-series/documents/sps-siot-mpr-series-datasheet-32332628-ciid-172626.pdf
 */

#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/stddef.h>

#include <linux/iio/iio.h>

#include "mprls0025pa.h"

static int mpr_spi_xfer(struct mpr_data *data)
{
	struct spi_device *spi = to_spi_device(data->dev);
	int ret, i;
	u8 cmd[7] = {MPR_CMD_SYNC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	int nloops = 10;
	struct spi_transfer xfer = {
		.tx_buf = cmd,
		.rx_buf = data->buffer,
		.len = 3,
	};

	ret = spi_sync_transfer(spi, &xfer, 1);
	if (ret)
		return ret;

	reinit_completion(&data->completion);
	cmd[0] = MPR_CMD_NOP;

	if (data->irq > 0) {
		ret = wait_for_completion_timeout(&data->completion, HZ);
		if (!ret) {
			dev_err(data->dev, "timeout while waiting for eoc irq\n");
			return -ETIMEDOUT;
		}
	} else {
		xfer.len = 1;
		/* wait until status indicates data is ready */
		for (i = 0; i < nloops; i++) {
			/*
			 * datasheet only says to wait at least 5 ms for the
			 * data but leave the maximum response time open
			 * --> let's try it nloops (10) times which seems to be
			 *     quite long
			 */
			usleep_range(5000, 10000);
			ret = spi_sync_transfer(spi, &xfer, 1);
			if (ret)
				return ret;
			if (data->buffer[0] == MPR_I2C_POWER)
				break;
		}
		if (i == nloops) {
			dev_err(data->dev, "timeout while reading\n");
			return -ETIMEDOUT;
		}
	}

	xfer.len = 7;
	ret = spi_sync_transfer(spi, &xfer, 1);
	if (ret)
		return ret;

	pr_info("got %02x %02x %02x %02x %02x %02x\n", data->buffer[1], data->buffer[2],
		data->buffer[3], data->buffer[4], data->buffer[5], data->buffer[6]);

	if (data->buffer[0] != MPR_I2C_POWER) {
		dev_err(data->dev,
			"unexpected status byte %02x\n", data->buffer[0]);
		return -ETIMEDOUT;
	}

	return 0;
}

static int mpr_spi_probe(struct spi_device *spi)
{
	return mpr_common_probe(&spi->dev, mpr_spi_xfer);
}

static const struct of_device_id mpr_spi_match[] = {
	{ .compatible = "honeywell,mprls0025pa" },
	{}
};
MODULE_DEVICE_TABLE(of, mpr_spi_match);

static const struct spi_device_id mpr_spi_id[] = {
	{ "mprls0025pa" },
	{}
};
MODULE_DEVICE_TABLE(spi, mpr_spi_id);

static struct spi_driver mpr_spi_driver = {
	.driver = {
		.name = "mprls0025pa",
		.of_match_table = mpr_spi_match,
	},
	.probe = mpr_spi_probe,
	.id_table = mpr_spi_id,
};
module_spi_driver(mpr_spi_driver);

MODULE_AUTHOR("Petre Rodan <petre.rodan@subdimension.ro>");
MODULE_DESCRIPTION("Honeywell MPR pressure sensor spi driver");
MODULE_LICENSE("GPL");
MODULE_IMPORT_NS(IIO_HONEYWELL_MPRLS0025PA);
