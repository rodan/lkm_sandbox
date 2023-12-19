
## Linux kernel drivers and patches

available drivers | subsystem | specs | linux tree version | status
--- | --- | --- | --- | ---
[Honeywell HSC/SSC series](honeywell_hsc030pa) | iio | [datasheet 1](https://github.com/rodan/lkm_sandbox/blob/main/honeywell_hsc030pa/trustability-hsc-series.pdf) [2](https://github.com/rodan/lkm_sandbox/blob/main/honeywell_hsc030pa/trustability-ssc-series.pdf) | patched 6.7.0-rc4 | accepted upstream, probably part of 6.8
[Honeywell MPR series](honeywell_mprls0025pa) | iio | [datasheet](https://github.com/rodan/lkm_sandbox/blob/main/honeywell_mprls0025pa/micropressure-mpr-series.pdf)  | patched 6.7.0-rc4 | sent for upstream review

### compilation

as easy as

```
make
```

but if you're using a kernel based on the mainline 6.7 tree then make sure to read about the required [patch](linux-iio_property).

### device tree overlay

dts overlay files are provided for BeagleBone® Black devboards for each driver and each bus type. conversion to dtbo is provided via

```
make dtbs
```

