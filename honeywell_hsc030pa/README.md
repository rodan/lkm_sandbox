
## Honeywell TruStability™ HSC Series pressure sensor - iio device driver

Linux kernel driver for all digital variants of the [HSC family](https://sps.honeywell.com/us/en/products/advanced-sensing-technologies/healthcare-sensing/board-mount-pressure-sensors/trustability-hsc-series) of sensors.
both i2c and spi interface versions are covered by this library.

### device tree overlay contents

```
&i2cX {
        status = "okay";
        #address-cells = <1>;
        #size-cells = <0>;

        pressure@ADDR {
                compatible = "honeywell,hsc030pa";
                reg = <ADDR>;

                honeywell,transfer-function = <TRANSFER_FUNCTION_ID>;
                honeywell,pressure-triplet = "VARIANT";

                // in case of a custom chip, use NA as pressure-triplet
                // and populate pmin-pascal and pmax-pascal
                // with the range limits converted into pascals
                //honeywell,pressure-triplet = "NA";
                //honeywell,pmin-pascal = <0>;
                //honeywell,pmax-pascal = <206850>;

                //vdd-supply = <&foo>;
                status = "okay";
        };
};
```

where ```ADDR``` is the assigned i2c address: either ```0x28```, ```0x38```, ```0x48```, ```0x58```, ```0x68```, ```0x78```, ```0x88``` or ```0x98```.

The transfer function limits define the raw output of the sensor at a given pressure input.

```TRANSFER_FUNCTION_ID``` | nomenclature | info
--- | --- | ---
0 | A | 10% to 90% of 2^14 counts
1 | B | 5% to 95% of 2^14 counts
2 | C | 5% to 85% of 2^14 counts
3 | F | 4% to 94% of 2^14 counts


```VARIANT``` defines the pressure range as specified in the sensor's name and is one of: 001BA 1.6BA 2.5BA 004BA 006BA 010BA 1.6MD 2.5MD 004MD 006MD 010MD 016MD 025MD 040MD 060MD 100MD 160MD 250MD 400MD 600MD 001BD 1.6BD 2.5BD 004BD 2.5MG 004MG 006MG 010MG 016MG 025MG 040MG 060MG 100MG 160MG 250MG 400MG 600MG 001BG 1.6BG 2.5BG 004BG 006BG 010BG 100KA 160KA 250KA 400KA 600KA 001GA 160LD 250LD 400LD 600LD 001KD 1.6KD 2.5KD 004KD 006KD 010KD 016KD 025KD 040KD 060KD 100KD 160KD 250KD 400KD 250LG 400LG 600LG 001KG 1.6KG 2.5KG 004KG 006KG 010KG 016KG 025KG 040KG 060KG 100KG 160KG 250KG 400KG 600KG 001GG 015PA 030PA 060PA 100PA 150PA 0.5ND 001ND 002ND 004ND 005ND 010ND 020ND 030ND 001PD 005PD 015PD 030PD 060PD 001NG 002NG 004NG 005NG 010NG 020NG 030NG 001PG 005PG 015PG 030PG 060PG 100PG 150PG

please consult the chip nomenclature in the datasheet.

in case it's a custom chip with a different measurement range, then set ```NA``` (Not Available) as VARIANT and provide the limits:

```
        hsc@ADDR {
                status = "okay";
                compatible = "honeywell,hsc030pa";
                reg = <ADDR>;
                honeywell,transfer-function = <TRANSFER_FUNCTION_ID>;
                honeywell,pressure-triplet = "NA";
                honeywell,pmin-pascal = <(-4000000)>;
                honeywell,pmax-pascal = <200000>;
        };
```

### sysfs-based user-space interface

iio_info output

```
iio_info version: 0.25 (git tag:v0.25)
Libiio version: 0.25 (git tag: v0.25) backends: local xml ip usb
IIO context created with local backend.
Backend version: 0.25 (git tag: v0.25)
Backend description string: Linux beagle 6.7.0-rc2+ #2 PREEMPT Fri Nov 24 09:41:10 -00 2023 armv7l
IIO context has 2 attributes:
        local,kernel: 6.7.0-rc2+
        uri: local:
IIO context has 2 devices:
        iio:device0: hsc030pa
                2 channels found:
                        temp:  (input)
                        3 channel-specific attributes found:
                                attr  0: offset value: -511.749774830
                                attr  1: raw value: 755
                                attr  2: scale value: 97.703957
                        pressure:  (input)
                        3 channel-specific attributes found:
                                attr  0: offset value: -1638.000000000
                                attr  1: raw value: 7727
                                attr  2: scale value: 0.015780439
                1 device-specific attributes found:
                                attr  0: waiting_for_supplier value: 0
                No trigger on this device
```

```(double) (raw + offset) * scale``` provides the pressure in KPa and temperature in milli degrees C, as per the IIO ABI requirements.


