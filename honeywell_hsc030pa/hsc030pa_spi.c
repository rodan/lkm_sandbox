// SPDX-License-Identifier: GPL-2.0-only
/*
 * Honeywell TruStability HSC Series pressure/temperature sensor
 *
 * Copyright (c) 2023 Petre Rodan <petre.rodan@subdimension.ro>
 *
 * Datasheet: https://prod-edam.honeywell.com/content/dam/honeywell-edam/sps/siot/en-us/products/sensors/pressure-sensors/board-mount-pressure-sensors/trustability-hsc-series/documents/sps-siot-trustability-hsc-series-high-accuracy-board-mount-pressure-sensors-50099148-a-en-ciid-151133.pdf
 * Datasheet: https://prod-edam.honeywell.com/content/dam/honeywell-edam/sps/siot/en-us/products/sensors/pressure-sensors/common/documents/sps-siot-sleep-mode-technical-note-008286-1-en-ciid-155793.pdf
 */

#include <linux/delay.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/stddef.h>

#include "hsc030pa.h"

static int hsc_spi_recv(struct hsc_data *data)
{
	struct spi_device *spi = to_spi_device(data->dev);
	struct spi_transfer xfer = {
		.tx_buf = NULL,
		.rx_buf = NULL,
		.len = 0,
	};
	u16 orig_cs_setup_value;
	u8 orig_cs_setup_unit;

	if (data->capabilities & HSC_CAP_SLEEP) {
		/*
		 * Send the Full Measurement Request (FMR) command on the CS
		 * line in order to wake up the sensor as per
		 * "Sleep Mode for Use with Honeywell Digital Pressure Sensors"
		 * technical note (consult the datasheet link in the header).
		 *
		 * These specifications require the CS line to be held asserted
		 * for at least 8µs without any payload being generated.
		 */
		orig_cs_setup_value = spi->cs_setup.value;
		orig_cs_setup_unit = spi->cs_setup.unit;
		spi->cs_setup.value = 8;
		spi->cs_setup.unit = SPI_DELAY_UNIT_USECS;
		/*
		 * Send a dummy 0-size packet so that CS gets toggled.
		 * Trying to manually call spi->controller->set_cs() instead
		 * does not work as expected during the second call.
		 */
		spi_sync_transfer(spi, &xfer, 1);
		spi->cs_setup.value = orig_cs_setup_value;
		spi->cs_setup.unit = orig_cs_setup_unit;
	}

	msleep_interruptible(HSC_RESP_TIME_MS);

	xfer.rx_buf = data->buffer;
	xfer.len = HSC_REG_MEASUREMENT_RD_SIZE;
	return spi_sync_transfer(spi, &xfer, 1);
}

static int hsc_spi_probe(struct spi_device *spi)
{
	return hsc_common_probe(&spi->dev, hsc_spi_recv);
}

static const struct of_device_id hsc_spi_match[] = {
	{ .compatible = "honeywell,hsc030pa" },
	{}
};
MODULE_DEVICE_TABLE(of, hsc_spi_match);

static const struct spi_device_id hsc_spi_id[] = {
	{ "hsc030pa" },
	{}
};
MODULE_DEVICE_TABLE(spi, hsc_spi_id);

static struct spi_driver hsc_spi_driver = {
	.driver = {
		.name = "hsc030pa",
		.of_match_table = hsc_spi_match,
	},
	.probe = hsc_spi_probe,
	.id_table = hsc_spi_id,
};
module_spi_driver(hsc_spi_driver);

MODULE_AUTHOR("Petre Rodan <petre.rodan@subdimension.ro>");
MODULE_DESCRIPTION("Honeywell HSC and SSC pressure sensor spi driver");
MODULE_LICENSE("GPL");
MODULE_IMPORT_NS(IIO_HONEYWELL_HSC030PA);
