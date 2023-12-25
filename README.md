
## Linux kernel drivers and patches

available drivers | subsystem | specs | tree version | status
--- | --- | --- | --- | ---
[Honeywell ABP series](honeywell_abp060mg) | iio | [datasheet 1](https://github.com/rodan/lkm_sandbox/blob/main/datasheet/basic-abp-series.pdf) | 6.7.0-rc6 | under development
[Honeywell HSC/SSC series](honeywell_hsc030pa) | iio | [datasheet 1](https://github.com/rodan/lkm_sandbox/blob/main/datasheet/trustability-hsc-series.pdf) [2](https://github.com/rodan/lkm_sandbox/blob/main/datasheet/trustability-ssc-series.pdf) | patched 6.7.0-rc6 | accepted upstream, probably part of 6.8
[Honeywell MPR series](honeywell_mprls0025pa) | iio | [datasheet](https://github.com/rodan/lkm_sandbox/blob/main/datasheet/micropressure-mpr-series.pdf)  | patched 6.7.0-rc6 | sent for upstream review

### compilation

all drivers are provided as out-of-tree source files so compilation is as easy as

```
make
```

if you're using a kernel based on the mainline 6.7 tree then make sure to read about the required [patch](linux-iio_property).

### device tree overlay

dts overlay files are provided for BeagleBone® Black devboards for each driver and each bus type. conversion to dtbo is provided via

```
make dtbs
```

