
## Linux kernel drivers and patches

available drivers | linux tree version
--- | ---
[Honeywell HSC/SSC series](honeywell_hsc030pa) | patched 6.7.0-rc4
[Honeywell MPR series](honeywell_mprls0025pa) | patched 6.7.0-rc4

### compilation

as easy as

```
make
```

but if you're using the a kernel based on the mainline 6.7 tree then make sure to read about the required [patch](linux-iio_property).

### device tree overlay

dts overlay files are provided for BeagleBone® Black devboards for each driver and each bus type. conversion to dtbo is provided via

```
make dtbs
```

