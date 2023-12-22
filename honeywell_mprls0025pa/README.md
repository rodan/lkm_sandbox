
## Honeywell MicroPressure MPR Series sensor - iio device driver

Linux kernel driver for all digital variants of the [MPR family](https://sps.honeywell.com/us/en/products/advanced-sensing-technologies/healthcare-sensing/board-mount-pressure-sensors/micropressure-mpr-series) of sensors.
both i2c and spi interface versions are covered by this library.

```
 author:        Andreas Klinger <ak@it-klinger.de>
 license:       GNU GPLv2

modifications by Petre Rodan <petre.rodan@subdimension.ro>
```

changes include:

 * rewrite flow so that driver can use either i2c or spi as communication bus
 * add spi driver (tested on MPRLS0015PA0000SA)
 * add pressure-triplet property that automatically sets pmin, pmax
 * fix transfer-function enum binding typo
 * fix interrupt example in binding file


### device tree overlay contents

```
        pressure@0{
                compatible = "honeywell,mprls0025pa";
                reg = <0>;
                spi-max-frequency = <800000>;

                honeywell,transfer-function = <TRANSFER_FUNCTION_ID>;
                honeywell,pressure-triplet = "VARIANT";
                reset-gpios = <&gpio1 28 GPIO_ACTIVE_HIGH>;
                interrupt-parent = <&gpio0>;
                interrupts = <30 IRQ_TYPE_EDGE_RISING>;

                //vdd-supply = <&foo>;
                status = "okay";
        };
```

The transfer function limits define the raw output of the sensor at a given pressure input.

```TRANSFER_FUNCTION_ID``` | nomenclature | info
--- | --- | ---
1 | A | 10% to 90% of 2^24 counts
2 | B | 2.5% to 22.5% of 2^24 counts
3 | C | 20% to 80% of 2^24 counts


```VARIANT``` defines the pressure range as specified in the sensor's name and is one of: 0001BA, 01.6BA, 02.5BA, 0060MG, 0100MG, 0160MG, 0250MG, 0400MG, 0600MG, 0001BG, 01.6BG, 02.5BG, 0100KA, 0160KA, 0250KA, 0006KG, 0010KG, 0016KG, 0025KG, 0040KG, 0060KG, 0100KG, 0160KG, 0250KG, 0015PA, 0025PA, 0030PA, 0001PG, 0005PG, 0015PG, 0030PG, 0300YG

please consult the chip nomenclature in the datasheet.

in case it's a custom chip with a different measurement range, unset honeywell,pressure-triplet and provide the limits:

```
        pressure@ADDR {
                status = "okay";
                compatible = "honeywell,mprls0025pa";
                reg = <ADDR>;
                honeywell,transfer-function = <TRANSFER_FUNCTION_ID>;
                honeywell,pmin-pascal = <0>;
                honeywell,pmax-pascal = <206850>;
        };
```

### sysfs-based user-space interface

iio_info output

```
iio_info version: 0.25 (git tag:v0.25)
Libiio version: 0.25 (git tag: v0.25) backends: local xml ip usb
IIO context created with local backend.
Backend version: 0.25 (git tag: v0.25)
Backend description string: Linux beagle 6.7.0-rc4+ #2 PREEMPT Thu Dec  7 12:23:30 -00 2023 armv7l
IIO context has 2 attributes:
        local,kernel: 6.7.0-rc4+
        uri: local:
IIO context has 1 devices:
        iio:device0: mprls0025pa (buffer capable)
                2 channels found:
                        pressure:  (input, index: 0, format: le:S32/32>>0)
                        3 channel-specific attributes found:
                                attr  0: offset value: -1677722.000000000
                                attr  1: raw value: 14560437
                                attr  2: scale value: 0.007705465
                        timestamp:  (input, index: 1, format: le:S64/64>>0)
                2 device-specific attributes found:
                                attr  0: current_timestamp_clock value: realtime
                                attr  1: waiting_for_supplier value: 0
                2 buffer-specific attributes found:
                                attr  0: data_available value: 0
                                attr  1: direction value: in
ERROR: checking for trigger : Input/output error (5)
```

```(double) (raw + offset) * scale``` provides the pressure in Pa.

