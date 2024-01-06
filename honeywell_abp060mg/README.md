
## Honeywell TruStability™ ABP Series pressure sensor - iio device driver

Linux kernel driver for all digital variants of the [ABP family](https://sps.honeywell.com/us/en/products/advanced-sensing-technologies/healthcare-sensing/board-mount-pressure-sensors/basic-abp-series#resources) of sensors.
both i2c and spi interface versions are covered by this library.

ChangeLog:

 * refactor code so that driver can use either i2c or spi as communication bus
 * add spi driver (tested on ABPDANV030PGSA3)
 * add transfer-function property to get the chip capabilities
 * send wakeup sequence only if required by capabilities
 * add triggered buffer

### device tree overlay contents

```
&i2c2 {
	#address-cells = <1>;
	#size-cells = <0>;

	pinctrl-names = "default";
	pinctrl-0 = <&spi0_pins>;

	pressure@ADDR {
		compatible = "honeywell,VARIANT";
		reg = <ADDR>;
		spi-max-frequency = <800000>;
		honeywell,transfer-function = <TRANSFER_FUNCTION_ID>;
		vdd-supply = <&ldo4_reg>;
                status = "okay";
	};
};
```

where ```ADDR``` is the assigned i2c address: either ```0x08```, ```0x18```, ```0x28```, ```0x38```, ```0x48```, ```0x58```, ```0x68``` or ```0x78```.

The transfer function limits define the raw output of the sensor at a given pressure input and the chip's capabilities

```TRANSFER_FUNCTION_ID``` | nomenclature | info | capabilities
--- | --- | --- | ---
0 | A | 10% to 90% of 2^14 counts | none
1 | D | 10% to 90% of 2^14 counts | temperature, sleep mode
2 | S | 10% to 90% of 2^14 counts | sleep mode
3 | T | 10% to 90% of 2^14 counts | temperature

in case it's a custom chip with a different measurement range then the limits can be set via pmin-pascal, pmax-pascal

```
        pressure@ADDR {
                compatible = "honeywell,VARIANT";
                reg = <ADDR>;
                honeywell,transfer-function = <TRANSFER_FUNCTION_ID>;
                honeywell,pmin-pascal = <(-4000000)>;
                honeywell,pmax-pascal = <200000>;
                status = "okay";
        };
```

### sysfs-based user-space interface

iio_info output

```
iio_info version: 0.25 (git tag:v0.25)
Libiio version: 0.25 (git tag: v0.25) backends: local xml ip usb
IIO context created with local backend.
Backend version: 0.25 (git tag: v0.25)
Backend description string: Linux beagle 6.7.0-rc6+ #3 PREEMPT Tue Jan  2 17:12:38 -00 2024 armv7l
IIO context has 2 attributes:
        local,kernel: 6.7.0-rc6+
        uri: local:
IIO context has 2 devices:
        iio:device0: abp030pg (buffer capable)
                3 channels found:
                        pressure:  (input, index: 0, format: be:u14/16>>0)
                        3 channel-specific attributes found:
                                attr  0: offset value: -1638.000000
                                attr  1: raw value: 7791
                                attr  2: scale value: 0.015779905
                        temp:  (input, index: 1, format: be:u11/16>>5)
                        3 channel-specific attributes found:
                                attr  0: offset value: -511.749774830
                                attr  1: raw value: 761
                                attr  2: scale value: 97.703957
                        timestamp:  (input, index: 2, format: le:S64/64>>0)
                2 device-specific attributes found:
                                attr  0: current_timestamp_clock value: realtime
                                attr  1: waiting_for_supplier value: 0
                2 buffer-specific attributes found:
                                attr  0: data_available value: 0
                                attr  1: direction value: in
```

```(double) (raw + offset) * scale``` provides the pressure in KPa and temperature in milli degrees C, as per the IIO ABI requirements.


