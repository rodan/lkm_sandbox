
## Honeywell TruStability™ HSC Series pressure sensor - iio device driver

Linux kernel driver for all variants of the [HSC family](https://sps.honeywell.com/us/en/products/advanced-sensing-technologies/healthcare-sensing/board-mount-pressure-sensors/trustability-hsc-series) of sensors.
for the time being only the i2c version of the sensors is supported.

### device tree overlay contents

```
&i2cX {
        status = "okay";

        clock-frequency = <400000>;

        #address-cells = <1>;
        #size-cells = <0>;

        hsc@ADDR {
                status = "okay";
                compatible = "honeywell,VARIANT";
                reg = <ADDR>;
        };
};
```

where ```ADDR``` is the assigned i2c address: either ```0x28```, ```0x38```, ```0x48```, ```0x58```, ```0x68```, ```0x78```, ```0x88``` or ```0x98```.

and ```VARIANT``` defines the pressure range and is one of 001ba 1.6ba 2.5ba 004ba 006ba 010ba 1.6md 2.5md 004md 006md 010md 016md 025md 040md 060md 100md 160md 250md 400md 600md 001bd 1.6bd 2.5bd 004bd 2.5mg 004mg 006mg 010mg 016mg 025mg 040mg 060mg 100mg 160mg 250mg 400mg 600mg 001bg 1.6bg 2.5bg 004bg 006bg 010bg 100ka 160ka 250ka 400ka 600ka 001ga 160ld 250ld 400ld 600ld 001kd 1.6kd 2.5kd 004kd 006kd 010kd 016kd 025kd 040kd 060kd 100kd 160kd 250kd 400kd 250lg 400lg 600lg 001kg 1.6kg 2.5kg 004kg 006kg 010kg 016kg 025kg 040kg 060kg 100kg 160kg 250kg 400kg 600kg 001gg 015pa 030pa 060pa 100pa 150pa 0.5nd 001nd 002nd 004nd 005nd 010nd 020nd 030nd 001pd 005pd 015pd 030pd 060pd 001ng 002ng 004ng 005ng 010ng 020ng 030ng 001pg 005pg 015pg 030pg 060pg 100pg 150pg

please consult the chip nomenclature in the datasheet.

### sysfs-based user-space interface

iio_info output

```
iio_info version: 0.25 (git tag:v0.25)
Libiio version: 0.25 (git tag: v0.25) backends: local xml ip usb
IIO context created with local backend.
Backend version: 0.25 (git tag: v0.25)
Backend description string: Linux beagle 6.1.38+ #3 PREEMPT Tue Oct 10 19:39:56 -00 2023 armv7l
IIO context has 2 attributes:
        local,kernel: 6.1.38+
        uri: local:
IIO context has 1 devices:
        iio:device0: hsc030pa
                2 channels found:
                        temp:  (input)
                        3 channel-specific attributes found:
                                attr  0: offset value: -511.749774830
                                attr  1: raw value: 756
                                attr  2: scale value: 97.703957
                        pressure:  (input)
                        3 channel-specific attributes found:
                                attr  0: offset value: -1638.000000000
                                attr  1: raw value: 7787
                                attr  2: scale value: 0.015781643
                1 device-specific attributes found:
                                attr  0: waiting_for_supplier value: 0
                No trigger on this device
```

```(double) (raw + offset) * scale``` provides the pressure in KPa and temperature in mC, as per the IIO ABI requirements.


