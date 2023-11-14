/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Honeywell TruStability HSC Series pressure/temperature sensor
 *
 * Copyright (c) 2023 Petre Rodan <2b4eda@subdimension.ro>
 *
 */

#ifndef _HONEYWELL_HSC_H
#define _HONEYWELL_HSC_H

#define  HSC_REG_MEASUREMENT_RD_SIZE  4         // get all conversions in one go since transfers are not address-based
#define            HSC_RANGE_STR_LEN  6

struct hsc_chip_data;

struct hsc_data {
	struct i2c_client *client;
	const struct hsc_chip_data *chip;
	struct mutex lock;                      // lock protecting chip reads
	int (*xfer)(struct hsc_data * data);    // function that implements the chip reads
	bool is_valid;                          // false if last transfer has failed
	unsigned long last_update;              // time of last successful conversion
	u8 buffer[HSC_REG_MEASUREMENT_RD_SIZE]; // raw conversion data
	char range_str[HSC_RANGE_STR_LEN];	// range as defined by the chip nomenclature - ie "030PA" or "NA"
	s32 pmin;                               // min pressure limit
	s32 pmax;                               // max pressure limit
	u32 outmin;                             // minimum raw pressure in counts (based on transfer function)
	u32 outmax;                             // maximum raw pressure in counts (based on transfer function)
	u32 function;                           // transfer function
	s64 p_scale;                            // pressure scale
	s32 p_scale_nano;                       // pressure scale, decimal places
	s64 p_offset;                           // pressure offset
	s32 p_offset_nano;                      // pressure offset, decimal places
};

struct hsc_chip_data {
	bool (*valid)(struct hsc_data * data);  // function that checks the two status bits
	const struct iio_chan_spec *channels;   // channel specifications
	u8 num_channels;                        // pressure and temperature channels
};

enum hsc_func_id {
	HSC_FUNCTION_A,
	HSC_FUNCTION_B,
	HSC_FUNCTION_C,
	HSC_FUNCTION_F
};

enum hsc_variant {
	 HSC, SSC,
         HSC001BA, HSC1_6BA, HSC2_5BA, HSC004BA, HSC006BA, HSC010BA,
         HSC1_6MD, HSC2_5MD, HSC004MD, HSC006MD, HSC010MD, HSC016MD,
         HSC025MD, HSC040MD, HSC060MD, HSC100MD, HSC160MD, HSC250MD,
         HSC400MD, HSC600MD, HSC001BD, HSC1_6BD, HSC2_5BD, HSC004BD,
         HSC2_5MG, HSC004MG, HSC006MG, HSC010MG, HSC016MG, HSC025MG,
         HSC040MG, HSC060MG, HSC100MG, HSC160MG, HSC250MG, HSC400MG,
         HSC600MG, HSC001BG, HSC1_6BG, HSC2_5BG, HSC004BG, HSC006BG,
         HSC010BG, HSC100KA, HSC160KA, HSC250KA, HSC400KA, HSC600KA,
         HSC001GA, HSC160LD, HSC250LD, HSC400LD, HSC600LD, HSC001KD,
         HSC1_6KD, HSC2_5KD, HSC004KD, HSC006KD, HSC010KD, HSC016KD,
         HSC025KD, HSC040KD, HSC060KD, HSC100KD, HSC160KD, HSC250KD,
         HSC400KD, HSC250LG, HSC400LG, HSC600LG, HSC001KG, HSC1_6KG,
         HSC2_5KG, HSC004KG, HSC006KG, HSC010KG, HSC016KG, HSC025KG,
         HSC040KG, HSC060KG, HSC100KG, HSC160KG, HSC250KG, HSC400KG,
         HSC600KG, HSC001GG, HSC015PA, HSC030PA, HSC060PA, HSC100PA,
         HSC150PA, HSC0_5ND, HSC001ND, HSC002ND, HSC004ND, HSC005ND,
         HSC010ND, HSC020ND, HSC030ND, HSC001PD, HSC005PD, HSC015PD,
         HSC030PD, HSC060PD, HSC001NG, HSC002NG, HSC004NG, HSC005NG,
         HSC010NG, HSC020NG, HSC030NG, HSC001PG, HSC005PG, HSC015PG,
         HSC030PG, HSC060PG, HSC100PG, HSC150PG,
};

int hsc_probe(struct iio_dev *indio_dev, struct device *dev,
		 const char *name, int type);
void hsc_remove(struct iio_dev *indio_dev);

#endif
