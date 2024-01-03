// SPDX-License-Identifier: GPL-2.0-only
/*
 * Honeywell TruStability ABP Series pressure/temperature sensor
 *
 * Copyright (c) 2024 Petre Rodan <petre.rodan@subdimension.ro>
 *
 * Datasheet: https://prod-edam.honeywell.com/content/dam/honeywell-edam/sps/siot/en-us/products/sensors/pressure-sensors/board-mount-pressure-sensors/trustability-hsc-series/documents/sps-siot-trustability-hsc-series-high-accuracy-board-mount-pressure-sensors-50099148-a-en-ciid-151133.pdf
 * Datasheet: https://prod-edam.honeywell.com/content/dam/honeywell-edam/sps/siot/en-us/products/sensors/pressure-sensors/common/documents/sps-siot-sleep-mode-technical-note-008286-1-en-ciid-155793.pdf
 */

#include <linux/delay.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/spi/spi.h>

#include "abp060mg.h"

#define ABP_FMR_INTERVAL_US  1000

static int abp060mg_spi_recv(struct abp_state *state)
{
	struct spi_device *spi = to_spi_device(state->dev);
	struct spi_transfer xfer = {
		.tx_buf = NULL,
		.rx_buf = NULL,
		.len = 0,
	};
	u16 orig_cs_setup_value;
	u8 orig_cs_setup_unit;

	if (state->func_spec->capabilities & ABP_CAP_SLEEP) {
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
		 * does not work as expected the second time.
		 */
		spi_sync_transfer(spi, &xfer, 1);
		spi->cs_setup.value = orig_cs_setup_value;
		spi->cs_setup.unit = orig_cs_setup_unit;

		msleep_interruptible(ABP_RESP_TIME_MS);
	}

	xfer.rx_buf = state->buffer;
	xfer.len = state->read_len;
	return spi_sync_transfer(spi, &xfer, 1);
}

static int abp060mg_spi_probe(struct spi_device *spi)
{
	const struct spi_device_id *id = spi_get_device_id(spi);

	if (!id) {
		return -EOPNOTSUPP;
	}

	return abp060mg_common_probe(&spi->dev, abp060mg_spi_recv,
				     id->driver_data, id->name, ABP_FLAG_NULL);
}

static const struct spi_device_id abp060mg_spi_id_table[] = {
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
MODULE_DEVICE_TABLE(spi, abp060mg_spi_id_table);

static struct spi_driver abp060mg_spi_driver = {
	.driver = {
		.name = "abp060mg",
	},
	.probe = abp060mg_spi_probe,
	.id_table = abp060mg_spi_id_table,
};
module_spi_driver(abp060mg_spi_driver);

MODULE_AUTHOR("Petre Rodan <petre.rodan@subdimension.ro>");
MODULE_DESCRIPTION("Honeywell ABP pressure sensor spi driver");
MODULE_LICENSE("GPL");
MODULE_IMPORT_NS(IIO_HONEYWELL_ABP060MG);
