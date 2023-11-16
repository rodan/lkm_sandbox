// SPDX-License-Identifier: GPL-2.0-only
/*
 * Honeywell TruStability HSC Series pressure/temperature sensor
 *
 * Copyright (c) 2023 Petre Rodan <2b4eda@subdimension.ro>
 *
 * Datasheet: https://prod-edam.honeywell.com/content/dam/honeywell-edam/sps/siot/en-us/products/sensors/pressure-sensors/board-mount-pressure-sensors/trustability-hsc-series/documents/sps-siot-trustability-hsc-series-high-accuracy-board-mount-pressure-sensors-50099148-a-en-ciid-151133.pdf?download=false
 */

#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/init.h>
#include <linux/math64.h>
#include <linux/units.h>
#include <linux/mod_devicetable.h>
#include <linux/printk.h>

#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>

#include "honeywell_hsc.h"

struct hsc_func_spec {
	u32 output_min;
	u32 output_max;
};

static const struct hsc_func_spec hsc_func_spec[] = {
	[HSC_FUNCTION_A] = {.output_min = 1638, .output_max = 14746}, // 10% - 90% of 2^14
	[HSC_FUNCTION_B] = {.output_min =  819, .output_max = 15565}, //  5% - 95% of 2^14
	[HSC_FUNCTION_C] = {.output_min =  819, .output_max = 13926}, //  5% - 85% of 2^14
	[HSC_FUNCTION_F] = {.output_min =  655, .output_max = 15401}, //  4% - 94% of 2^14
};

// pressure range for current chip based on the nomenclature
struct hsc_range_config {
	char name[HSC_RANGE_STR_LEN];	// 5-char string that defines the range - ie "030PA"
	s32 pmin;		// minimal pressure in pascals
	s32 pmax;		// maximum pressure in pascals
};

// all min max limits have been converted to pascals
// code generated by scripts/parse_variants_table.sh
static const struct hsc_range_config hsc_range_config[] = {
	{ .name = "001BA", .pmin =       0, .pmax =  100000 },
	{ .name = "1.6BA", .pmin =       0, .pmax =  160000 },
	{ .name = "2.5BA", .pmin =       0, .pmax =  250000 },
	{ .name = "004BA", .pmin =       0, .pmax =  400000 },
	{ .name = "006BA", .pmin =       0, .pmax =  600000 },
	{ .name = "010BA", .pmin =       0, .pmax = 1000000 },
	{ .name = "1.6MD", .pmin =    -160, .pmax =     160 },
	{ .name = "2.5MD", .pmin =    -250, .pmax =     250 },
	{ .name = "004MD", .pmin =    -400, .pmax =     400 },
	{ .name = "006MD", .pmin =    -600, .pmax =     600 },
	{ .name = "010MD", .pmin =   -1000, .pmax =    1000 },
	{ .name = "016MD", .pmin =   -1600, .pmax =    1600 },
	{ .name = "025MD", .pmin =   -2500, .pmax =    2500 },
	{ .name = "040MD", .pmin =   -4000, .pmax =    4000 },
	{ .name = "060MD", .pmin =   -6000, .pmax =    6000 },
	{ .name = "100MD", .pmin =  -10000, .pmax =   10000 },
	{ .name = "160MD", .pmin =  -16000, .pmax =   16000 },
	{ .name = "250MD", .pmin =  -25000, .pmax =   25000 },
	{ .name = "400MD", .pmin =  -40000, .pmax =   40000 },
	{ .name = "600MD", .pmin =  -60000, .pmax =   60000 },
	{ .name = "001BD", .pmin = -100000, .pmax =  100000 },
	{ .name = "1.6BD", .pmin = -160000, .pmax =  160000 },
	{ .name = "2.5BD", .pmin = -250000, .pmax =  250000 },
	{ .name = "004BD", .pmin = -400000, .pmax =  400000 },
	{ .name = "2.5MG", .pmin =       0, .pmax =     250 },
	{ .name = "004MG", .pmin =       0, .pmax =     400 },
	{ .name = "006MG", .pmin =       0, .pmax =     600 },
	{ .name = "010MG", .pmin =       0, .pmax =    1000 },
	{ .name = "016MG", .pmin =       0, .pmax =    1600 },
	{ .name = "025MG", .pmin =       0, .pmax =    2500 },
	{ .name = "040MG", .pmin =       0, .pmax =    4000 },
	{ .name = "060MG", .pmin =       0, .pmax =    6000 },
	{ .name = "100MG", .pmin =       0, .pmax =   10000 },
	{ .name = "160MG", .pmin =       0, .pmax =   16000 },
	{ .name = "250MG", .pmin =       0, .pmax =   25000 },
	{ .name = "400MG", .pmin =       0, .pmax =   40000 },
	{ .name = "600MG", .pmin =       0, .pmax =   60000 },
	{ .name = "001BG", .pmin =       0, .pmax =  100000 },
	{ .name = "1.6BG", .pmin =       0, .pmax =  160000 },
	{ .name = "2.5BG", .pmin =       0, .pmax =  250000 },
	{ .name = "004BG", .pmin =       0, .pmax =  400000 },
	{ .name = "006BG", .pmin =       0, .pmax =  600000 },
	{ .name = "010BG", .pmin =       0, .pmax = 1000000 },
	{ .name = "100KA", .pmin =       0, .pmax =  100000 },
	{ .name = "160KA", .pmin =       0, .pmax =  160000 },
	{ .name = "250KA", .pmin =       0, .pmax =  250000 },
	{ .name = "400KA", .pmin =       0, .pmax =  400000 },
	{ .name = "600KA", .pmin =       0, .pmax =  600000 },
	{ .name = "001GA", .pmin =       0, .pmax = 1000000 },
	{ .name = "160LD", .pmin =    -160, .pmax =     160 },
	{ .name = "250LD", .pmin =    -250, .pmax =     250 },
	{ .name = "400LD", .pmin =    -400, .pmax =     400 },
	{ .name = "600LD", .pmin =    -600, .pmax =     600 },
	{ .name = "001KD", .pmin =   -1000, .pmax =    1000 },
	{ .name = "1.6KD", .pmin =   -1600, .pmax =    1600 },
	{ .name = "2.5KD", .pmin =   -2500, .pmax =    2500 },
	{ .name = "004KD", .pmin =   -4000, .pmax =    4000 },
	{ .name = "006KD", .pmin =   -6000, .pmax =    6000 },
	{ .name = "010KD", .pmin =  -10000, .pmax =   10000 },
	{ .name = "016KD", .pmin =  -16000, .pmax =   16000 },
	{ .name = "025KD", .pmin =  -25000, .pmax =   25000 },
	{ .name = "040KD", .pmin =  -40000, .pmax =   40000 },
	{ .name = "060KD", .pmin =  -60000, .pmax =   60000 },
	{ .name = "100KD", .pmin = -100000, .pmax =  100000 },
	{ .name = "160KD", .pmin = -160000, .pmax =  160000 },
	{ .name = "250KD", .pmin = -250000, .pmax =  250000 },
	{ .name = "400KD", .pmin = -400000, .pmax =  400000 },
	{ .name = "250LG", .pmin =       0, .pmax =     250 },
	{ .name = "400LG", .pmin =       0, .pmax =     400 },
	{ .name = "600LG", .pmin =       0, .pmax =     600 },
	{ .name = "001KG", .pmin =       0, .pmax =    1000 },
	{ .name = "1.6KG", .pmin =       0, .pmax =    1600 },
	{ .name = "2.5KG", .pmin =       0, .pmax =    2500 },
	{ .name = "004KG", .pmin =       0, .pmax =    4000 },
	{ .name = "006KG", .pmin =       0, .pmax =    6000 },
	{ .name = "010KG", .pmin =       0, .pmax =   10000 },
	{ .name = "016KG", .pmin =       0, .pmax =   16000 },
	{ .name = "025KG", .pmin =       0, .pmax =   25000 },
	{ .name = "040KG", .pmin =       0, .pmax =   40000 },
	{ .name = "060KG", .pmin =       0, .pmax =   60000 },
	{ .name = "100KG", .pmin =       0, .pmax =  100000 },
	{ .name = "160KG", .pmin =       0, .pmax =  160000 },
	{ .name = "250KG", .pmin =       0, .pmax =  250000 },
	{ .name = "400KG", .pmin =       0, .pmax =  400000 },
	{ .name = "600KG", .pmin =       0, .pmax =  600000 },
	{ .name = "001GG", .pmin =       0, .pmax = 1000000 },
	{ .name = "015PA", .pmin =       0, .pmax =  103425 },
	{ .name = "030PA", .pmin =       0, .pmax =  206850 },
	{ .name = "060PA", .pmin =       0, .pmax =  413700 },
	{ .name = "100PA", .pmin =       0, .pmax =  689500 },
	{ .name = "150PA", .pmin =       0, .pmax = 1034250 },
	{ .name = "0.5ND", .pmin =    -125, .pmax =     125 },
	{ .name = "001ND", .pmin =    -249, .pmax =     249 },
	{ .name = "002ND", .pmin =    -498, .pmax =     498 },
	{ .name = "004ND", .pmin =    -996, .pmax =     996 },
	{ .name = "005ND", .pmin =   -1245, .pmax =    1245 },
	{ .name = "010ND", .pmin =   -2491, .pmax =    2491 },
	{ .name = "020ND", .pmin =   -4982, .pmax =    4982 },
	{ .name = "030ND", .pmin =   -7473, .pmax =    7473 },
	{ .name = "001PD", .pmin =   -6895, .pmax =    6895 },
	{ .name = "005PD", .pmin =  -34475, .pmax =   34475 },
	{ .name = "015PD", .pmin = -103425, .pmax =  103425 },
	{ .name = "030PD", .pmin = -206850, .pmax =  206850 },
	{ .name = "060PD", .pmin = -413700, .pmax =  413700 },
	{ .name = "001NG", .pmin =       0, .pmax =     249 },
	{ .name = "002NG", .pmin =       0, .pmax =     498 },
	{ .name = "004NG", .pmin =       0, .pmax =     996 },
	{ .name = "005NG", .pmin =       0, .pmax =    1245 },
	{ .name = "010NG", .pmin =       0, .pmax =    2491 },
	{ .name = "020NG", .pmin =       0, .pmax =    4982 },
	{ .name = "030NG", .pmin =       0, .pmax =    7473 },
	{ .name = "001PG", .pmin =       0, .pmax =    6895 },
	{ .name = "005PG", .pmin =       0, .pmax =   34475 },
	{ .name = "015PG", .pmin =       0, .pmax =  103425 },
	{ .name = "030PG", .pmin =       0, .pmax =  206850 },
	{ .name = "060PG", .pmin =       0, .pmax =  413700 },
	{ .name = "100PG", .pmin =       0, .pmax =  689500 },
	{ .name = "150PG", .pmin =       0, .pmax = 1034250 },
	{}
};

/*
 * the first two bits from the first byte contain a status code
 *
 * 00 - normal operation, valid data
 * 01 - device in hidden factory command mode
 * 10 - stale data
 * 11 - diagnostic condition
 *
 * function returns 1 only if both bits are zero
 */
static bool hsc_measurement_is_valid(struct hsc_data *data)
{
	if (data->buffer[0] & 0xc0)
		return 0;

	return 1;
}

static int hsc_get_measurement(struct hsc_data *data)
{
	const struct hsc_chip_data *chip = data->chip;
	int ret;

	/* don't bother sensor more than once a second */
	if (!time_after(jiffies, data->last_update + HZ)) {
		return data->is_valid ? 0 : -EAGAIN;
	}

	data->is_valid = false;
	data->last_update = jiffies;

	ret = data->xfer(data);

	if (ret < 0)
		return ret;

	pr_info("recvd %02x %02x %02x %02x, status %02x\n", data->buffer[0],
		data->buffer[1], data->buffer[2], data->buffer[3],
		chip->valid(data));

	ret = chip->valid(data);
	if (!ret)
		return -EAGAIN;

	data->is_valid = true;

	return 0;
}

/*
4 bytes are read, the dissection looks like

.  0  .  1  .  2  .  3  .  4  .  5  .  6  .  7  .
byte 0:
|  s1 |  s0 | b13 | b12 | b11 | b10 |  b9 |  b8 |
| status    | bridge data (pressure) MSB        |
byte 1:
|  b7 |  b6 |  b5 |  b4 |  b3 |  b2 |  b1 |  b0 |
| bridge data (pressure) LSB                    |
byte 2:
| t10 |  t9 |  t8 |  t7 |  t6 |  t5 |  t4 |  t3 |
| temperature data MSB                          |
byte 3:
|  t2 |  t1 |  t0 |  X  |  X  |  X  |  X  |  X  |
| temperature LSB | ignore                      |

.  0  .  1  .  2  .  3  .  4  .  5  .  6  .  7  .

*/

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
			*val =
			    ((data->buffer[0] & 0x3f) << 8) + data->buffer[1];
			return IIO_VAL_INT;
		case IIO_TEMP:
			*val =
			    (data->buffer[2] << 3) +
			    ((data->buffer[3] & 0xe0) >> 5);
			ret = 0;
			if (!ret)
				return IIO_VAL_INT;
			break;
		default:
			return -EINVAL;
		}
		break;

/**
 *	IIO ABI expects
 *	value = (conv + offset) * scale
 *
 *	datasheet provides the following formula for determining the temperature
 *	temp[C] = conv * a + b
 *        where a = 200/2047; b = -50
 *
 *	temp[C] = (conv + (b/a)) * a * (1000)
 *      =>
 *	scale = a * 1000 = .097703957 * 1000 = 97.703957
 *	offset = b/a = -50 / .097703957 = -50000000 / 97704
 *
 *	based on the datasheet
 *	pressure = (conv - HSC_OUTPUT_MIN) * Q + Pmin =
 *	           ((conv - HSC_OUTPUT_MIN) + Pmin/Q) * Q
 *	=>
 *	scale = Q = (Pmax - Pmin) / (HSC_OUTPUT_MAX - HSC_OUTPUT_MIN)
 *	offset = Pmin/Q = Pmin * (HSC_OUTPUT_MAX - HSC_OUTPUT_MIN) / (Pmax - Pmin)
*/

	case IIO_CHAN_INFO_SCALE:
		switch (channel->type) {
		case IIO_TEMP:
			*val = 97;
			*val2 = 703957;
			return IIO_VAL_INT_PLUS_MICRO;
		case IIO_PRESSURE:
			*val = data->p_scale;
			*val2 = data->p_scale_nano;
			return IIO_VAL_INT_PLUS_NANO;
		default:
			return -EINVAL;
		}
		break;

	case IIO_CHAN_INFO_OFFSET:
		switch (channel->type) {
		case IIO_TEMP:
			*val = -50000000;
			*val2 = 97704;
			return IIO_VAL_FRACTIONAL;
		case IIO_PRESSURE:
			*val = data->p_offset;
			*val2 = data->p_offset_nano;
			return IIO_VAL_INT_PLUS_NANO;
		default:
			return -EINVAL;
		}
	}

	return ret;
}

static const struct iio_chan_spec hsc_channels[] = {
	{
	 .type = IIO_PRESSURE,
	 .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) |
	 BIT(IIO_CHAN_INFO_SCALE) | BIT(IIO_CHAN_INFO_OFFSET)
	 },
	{
	 .type = IIO_TEMP,
	 .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) |
	 BIT(IIO_CHAN_INFO_SCALE) | BIT(IIO_CHAN_INFO_OFFSET)
	 },
};

static const struct iio_info hsc_info = {
	.read_raw = hsc_read_raw,
};

static const struct hsc_chip_data hsc_chip = {
	.valid = hsc_measurement_is_valid,
	.channels = hsc_channels,
	.num_channels = ARRAY_SIZE(hsc_channels),
};

int hsc_probe(struct iio_dev *indio_dev, struct device *dev,
	      const char *name, int type)
{
	struct hsc_data *hsc;
	u64 tmp;
	int index;
	int found = 0;

	hsc = iio_priv(indio_dev);

	hsc->last_update = jiffies - HZ;
	hsc->chip = &hsc_chip;

	if (strcasecmp(hsc->range_str, "na") != 0) {
		// chip should be defined in the nomenclature
		for (index = 0; index < ARRAY_SIZE(hsc_range_config); index++) {
			if (strcasecmp
			    (hsc_range_config[index].name,
			     hsc->range_str) == 0) {
				pr_info("hsc found '%s' under id %d\n",
					hsc->range_str, index);
				hsc->pmin = hsc_range_config[index].pmin;
				hsc->pmax = hsc_range_config[index].pmax;
				found = 1;
				break;
			}
		}
		if (hsc->pmin == hsc->pmax || !found)
			return dev_err_probe(dev, -EINVAL,
					     "honeywell,range_str is invalid\n");
	}

	hsc->outmin = hsc_func_spec[hsc->function].output_min;
	hsc->outmax = hsc_func_spec[hsc->function].output_max;

	pr_info("hsc out %d - %d\n", hsc->outmin, hsc->outmax);

	// multiply with MICRO and then divide by NANO since the output needs
	// to be in KPa as per IIO ABI requirement
	tmp = div_s64(((s64) (hsc->pmax - hsc->pmin)) * MICRO,
		      (hsc->outmax - hsc->outmin));
	hsc->p_scale = div_s64_rem(tmp, NANO, &hsc->p_scale_nano);
	tmp =
	    div_s64(((s64) hsc->pmin * (s64) (hsc->outmax - hsc->outmin)) *
		    MICRO, hsc->pmax - hsc->pmin);
	hsc->p_offset =
	    div_s64_rem(tmp, NANO, &hsc->p_offset_nano) - hsc->outmin;

	mutex_init(&hsc->lock);
	indio_dev->name = name;
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->info = &hsc_info;
	indio_dev->channels = hsc->chip->channels;
	indio_dev->num_channels = hsc->chip->num_channels;

	return devm_iio_device_register(dev, indio_dev);
}
EXPORT_SYMBOL_NS(hsc_probe, IIO_HONEYWELL_HSC);

void hsc_remove(struct iio_dev *indio_dev)
{
	iio_device_unregister(indio_dev);
}
EXPORT_SYMBOL_NS(hsc_remove, IIO_HONEYWELL_HSC);

MODULE_AUTHOR("Petre Rodan <2b4eda@subdimension.ro>");
MODULE_DESCRIPTION("Honeywell HSC pressure sensor core driver");
MODULE_LICENSE("GPL");
