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

static int mpr_spi_xfer(struct mpr_data *data, const u8 cmd, const u8 pkt_len)
{
	struct spi_device *spi = to_spi_device(data->dev);
	u8 tx_buf[MPR_MEASUREMENT_RD_SIZE];
	struct spi_transfer xfer;

	if (pkt_len > MPR_MEASUREMENT_RD_SIZE)
		return -EOVERFLOW;

	tx_buf[0] = cmd;
	xfer.tx_buf = tx_buf;
	xfer.rx_buf = data->buffer;
	xfer.len = pkt_len;

	return spi_sync_transfer(spi, &xfer, 1);
}

static int mpr_spi_probe(struct spi_device *spi)
{
	return mpr_common_probe(&spi->dev, mpr_spi_xfer, mpr_spi_xfer, spi->irq);
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
