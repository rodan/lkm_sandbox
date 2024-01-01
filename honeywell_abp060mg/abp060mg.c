// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2016 - Marcin Malagowski <mrc@bourne.st>
 * Copyright (C) 2024 - Petre Rodan <petre.rodan@subdimension.ro>
 */

#include <linux/array_size.h>
#include <linux/bitfield.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/math64.h>
#include <linux/module.h>
#include <linux/property.h>
#include <linux/units.h>

#include "abp060mg.h"
#define ABP_ERROR_MASK        GENMASK(7, 6)
#define ABP_TEMPERATURE_MASK  GENMASK(15, 5)
#define ABP_PRESSURE_MASK     GENMASK(29, 16)

#define ABP060MG_ERROR_MASK   0xC000
#define ABP060MG_MIN_COUNTS   1638  /* = 0x0666 (10% of u14) */
#define ABP060MG_MAX_COUNTS   14746 /* = 0x399a (90% of u14) */
#define ABP060MG_NUM_COUNTS   (ABP060MG_MAX_COUNTS - ABP060MG_MIN_COUNTS)

struct abp_config {
	int min;
	int max;
};

static struct abp_config abp_config[] = {
	/* mbar & kPa variants */
	[ABP006KG] = { .min =       0, .max =     6000 },
	[ABP010KG] = { .min =       0, .max =    10000 },
	[ABP016KG] = { .min =       0, .max =    16000 },
	[ABP025KG] = { .min =       0, .max =    25000 },
	[ABP040KG] = { .min =       0, .max =    40000 },
	[ABP060KG] = { .min =       0, .max =    60000 },
	[ABP100KG] = { .min =       0, .max =   100000 },
	[ABP160KG] = { .min =       0, .max =   160000 },
	[ABP250KG] = { .min =       0, .max =   250000 },
	[ABP400KG] = { .min =       0, .max =   400000 },
	[ABP600KG] = { .min =       0, .max =   600000 },
	[ABP001GG] = { .min =       0, .max =  1000000 },
	[ABP006KD] = { .min =   -6000, .max =     6000 },
	[ABP010KD] = { .min =  -10000, .max =    10000 },
	[ABP016KD] = { .min =  -16000, .max =    16000 },
	[ABP025KD] = { .min =  -25000, .max =    25000 },
	[ABP040KD] = { .min =  -40000, .max =    40000 },
	[ABP060KD] = { .min =  -60000, .max =    60000 },
	[ABP100KD] = { .min = -100000, .max =   100000 },
	[ABP160KD] = { .min = -160000, .max =   160000 },
	[ABP250KD] = { .min = -250000, .max =   250000 },
	[ABP400KD] = { .min = -400000, .max =   400000 },
	/* psi variants (1 psi ~ 6895 Pa) */
	[ABP001PG] = { .min =       0, .max =     6895 },
	[ABP005PG] = { .min =       0, .max =    34474 },
	[ABP015PG] = { .min =       0, .max =   103421 },
	[ABP030PG] = { .min =       0, .max =   206843 },
	[ABP060PG] = { .min =       0, .max =   413686 },
	[ABP100PG] = { .min =       0, .max =   689476 },
	[ABP150PG] = { .min =       0, .max =  1034214 },
	[ABP001PD] = { .min =   -6895, .max =     6895 },
	[ABP005PD] = { .min =  -34474, .max =    34474 },
	[ABP015PD] = { .min = -103421, .max =   103421 },
	[ABP030PD] = { .min = -206843, .max =   206843 },
	[ABP060PD] = { .min = -413686, .max =   413686 },
};

enum abp_func_id {
	ABP_FUNCTION_A,
	ABP_FUNCTION_D,
	ABP_FUNCTION_S,
	ABP_FUNCTION_T,
};

static const struct iio_chan_spec abp060mg_p_channel[] = {
	{
		.type = IIO_PRESSURE,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW) |
			BIT(IIO_CHAN_INFO_OFFSET) | BIT(IIO_CHAN_INFO_SCALE),
	},
};

static const struct iio_chan_spec abp060mg_pt_channel[] = {
	{
		.type = IIO_PRESSURE,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW) |
				      BIT(IIO_CHAN_INFO_SCALE) |
				      BIT(IIO_CHAN_INFO_OFFSET),
	},
	{
		.type = IIO_TEMP,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW) |
				      BIT(IIO_CHAN_INFO_SCALE) |
				      BIT(IIO_CHAN_INFO_OFFSET),
	},
};

static bool abp060mg_conversion_is_valid(struct abp_state *state)
{
	return !(state->buffer[0] & ABP_ERROR_MASK);
}

static int abp060mg_get_measurement(struct abp_state *state)
{
	//const struct hsc_chip_data *chip = data->chip;
	int ret;

	ret = state->recv_cb(state);
	if (ret < 0)
		return ret;

	state->is_valid = abp060mg_conversion_is_valid(state);
	if (!state->is_valid)
		return -EAGAIN;

	return 0;
}

static int abp060mg_read_raw(struct iio_dev *indio_dev,
			struct iio_chan_spec const *chan, int *val,
			int *val2, long mask)
{
	struct abp_state *state = iio_priv(indio_dev);
	int ret;
	u32 recvd;

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		ret = abp060mg_get_measurement(state);
		if (ret)
			return ret;

		recvd = get_unaligned_be32(state->buffer);
		switch (chan->type) {
		case IIO_PRESSURE:
			*val = FIELD_GET(ABP_PRESSURE_MASK, recvd);
			return IIO_VAL_INT;
		case IIO_TEMP:
			*val = FIELD_GET(ABP_TEMPERATURE_MASK, recvd);
			return IIO_VAL_INT;
		default:
			return -EINVAL;
		}
	case IIO_CHAN_INFO_OFFSET:
		switch (chan->type) {
		case IIO_TEMP:
			*val = -50000000;
			*val2 = 97704;
			return IIO_VAL_FRACTIONAL;
		case IIO_PRESSURE:
			*val = state->p_offset;
			*val2 = state->p_offset_dec;
			return IIO_VAL_INT_PLUS_MICRO;
		default:
			return -EINVAL;
		}
		break;
	case IIO_CHAN_INFO_SCALE:
		switch (chan->type) {
		case IIO_TEMP:
			*val = 97;
			*val2 = 703957;
			return IIO_VAL_INT_PLUS_MICRO;
		case IIO_PRESSURE:
			*val = state->p_scale;
			*val2 = ABP060MG_NUM_COUNTS * 1000; /* to kPa */
			return IIO_VAL_FRACTIONAL;
		default:
			return -EINVAL;
		}
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static const struct iio_info abp060mg_info = {
	.read_raw = abp060mg_read_raw,
};

static bool abp060mg_sensor_has_temp(const u32 transfer_function)
{
	switch (transfer_function) {
	case ABP_FUNCTION_D:
	case ABP_FUNCTION_T:
		return true;
	default:
		return false;
	};
}

static bool abp060mg_sensor_has_sleep_mode(const u32 transfer_function)
{
	switch (transfer_function) {
	case ABP_FUNCTION_D:
	case ABP_FUNCTION_S:
		return true;
	default:
		return false;
	};
}

static void abp060mg_init_device(struct iio_dev *indio_dev, unsigned long id)
{
	struct abp_state *state = iio_priv(indio_dev);
	struct abp_config *cfg = &abp_config[id];
	s64 tmp;

	state->p_scale = cfg->max - cfg->min;
	state->pmin = cfg->min;
	state->pmax = cfg->max;
	state->outmin = ABP060MG_MIN_COUNTS;
	state->outmax = ABP060MG_MAX_COUNTS;

	tmp = div_s64(((s64)state->pmin * (s64)(state->outmax - state->outmin)) * MICRO,
		      state->pmax - state->pmin);
	tmp -= (s64)state->outmin * MICRO;
	state->p_offset = div_s64_rem(tmp, MICRO, &state->p_offset_dec);
}

int abp060mg_common_probe(struct device *dev, abp_recv_fn recv, const u32 cfg_id,
		     const u32 flags)
{
	struct iio_dev *indio_dev;
	struct abp_state *state;
	int ret;

	indio_dev = devm_iio_device_alloc(dev, sizeof(*state));
	if (!indio_dev)
		return -ENOMEM;

	state = iio_priv(indio_dev);

	state->recv_cb = recv;
	state->dev = dev;

	if (flags & ABP_FLAG_MREQ)
		state->mreq_len = 1;

	abp060mg_init_device(indio_dev, cfg_id);

	ret = device_property_read_u32(dev, "honeywell,transfer-function",
				       &state->function);
	if (ret)
		return dev_err_probe(dev, ret,
			    "honeywell,transfer-function could not be read\n");
	if (state->function > ABP_FUNCTION_T)
		return dev_err_probe(dev, -EINVAL,
				     "honeywell,transfer-function %d invalid\n",
				     state->function);

	device_property_read_u32(dev, "honeywell,pmin-pascal", &state->pmin);
	device_property_read_u32(dev, "honeywell,pmax-pascal", &state->pmax);

	if (state->pmin >= state->pmax)
		return dev_err_probe(dev, -EINVAL,
				     "pressure limits are invalid\n");

	indio_dev->name = dev_name(dev);
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->info = &abp060mg_info;

	if (abp060mg_sensor_has_temp(state->function)) {
		indio_dev->channels = abp060mg_pt_channel;
		indio_dev->num_channels = ARRAY_SIZE(abp060mg_pt_channel);
		state->read_len = 4;
	} else {
		indio_dev->channels = abp060mg_p_channel;
		indio_dev->num_channels = ARRAY_SIZE(abp060mg_p_channel);
		state->read_len = 2;
	}

	return devm_iio_device_register(dev, indio_dev);
}
EXPORT_SYMBOL_NS(abp060mg_common_probe, IIO_HONEYWELL_ABP060MG);

MODULE_AUTHOR("Marcin Malagowski <mrc@bourne.st>");
MODULE_DESCRIPTION("Honeywell ABP pressure sensor driver");
MODULE_LICENSE("GPL");
