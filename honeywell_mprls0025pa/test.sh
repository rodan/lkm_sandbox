#!/bin/bash

target='mprls0025pa'

rmmod "${target}_i2c" 2>/dev/null
rmmod "${target}_spi" 2>/dev/null
rmmod "${target}" 2>/dev/null

sleep 1

insmod "${target}.ko"
insmod "${target}_i2c.ko"
insmod "${target}_spi.ko"

sleep 1

iio_info

