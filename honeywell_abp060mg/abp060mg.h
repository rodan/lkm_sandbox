/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Honeywell ABP series Basic Board Mount Pressure Sensors
 *
 * Copyright (c) 2023 Petre Rodan <petre.rodan@subdimension.ro>
 */

#ifndef _ABP060MG_H
#define _ABP060MG_H

#include <linux/types.h>

#include <linux/iio/iio.h>

#define ABP_REG_MEASUREMENT_RD_SIZE 4
#define ABP_RESP_TIME_MS 40

/* flags accepted as argument to abp060mg_common_probe() */
#define ABP_FLAG_NULL    0
#define ABP_FLAG_MREQ    0x1

#define ABP_CAP_NULL     0x00
#define ABP_CAP_TEMP     0x01 /* sensor can provide temperature conversions */
#define ABP_CAP_SLEEP    0x02 /* sensor needs to be woken up */

struct device;

struct iio_chan_spec;
struct iio_dev;

struct abp_state;
struct abp_func_spec;

typedef int (*abp_recv_fn)(struct abp_state *);

enum abp_variant {
	/* gage [kPa] */
	ABP006KG, ABP010KG, ABP016KG, ABP025KG, ABP040KG, ABP060KG, ABP100KG,
	ABP160KG, ABP250KG, ABP400KG, ABP600KG, ABP001GG,
	/* differential [kPa] */
	ABP006KD, ABP010KD, ABP016KD, ABP025KD, ABP040KD, ABP060KD, ABP100KD,
	ABP160KD, ABP250KD, ABP400KD,
	/* gage [psi] */
	ABP001PG, ABP005PG, ABP015PG, ABP030PG, ABP060PG, ABP100PG, ABP150PG,
	/* differential [psi] */
	ABP001PD, ABP005PD, ABP015PD, ABP030PD, ABP060PD,
};

/**
 * struct abp_state
 * @dev: current device structure
 * @func_spec: transfer function specification
 * @recv_cb: function that implements the chip reads
 * @is_valid: true if last transfer has been validated
 * @mreq_len: measure request - 1 if one dummy byte needs to be sent to wake up
 *             sensor
 * @read_len: number of bytes to be read from sensor
 * @pmin: minimum measurable pressure limit
 * @pmax: maximum measurable pressure limit
 * @p_scale: pressure scale
 * @p_scale_dec: pressure scale, decimal places
 * @p_offset: pressure offset
 * @p_offset_dec: pressure offset, decimal places
 * @buffer: raw conversion data
 */
struct abp_state {
	struct device *dev;
	const struct abp_func_spec *func_spec;
	abp_recv_fn recv_cb;
	bool is_valid;
	int mreq_len;
	u8 read_len;
	s32 pmin;
	s32 pmax;
	s64 p_scale;
	s64 p_scale_dec;
	s64 p_offset;
	s32 p_offset_dec;
	u8 buffer[ABP_REG_MEASUREMENT_RD_SIZE] __aligned(IIO_DMA_MINALIGN);
};

struct abp_func_spec {
	u32 output_min;
	u32 output_max;
	u16 capabilities;
};

int abp060mg_common_probe(struct device *dev, abp_recv_fn recv, const u32 cfg_id,
			  const u32 flags);

#endif
