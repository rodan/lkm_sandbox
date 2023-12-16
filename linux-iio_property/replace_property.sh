#!/bin/bash

items="include/linux/property.h
drivers/base/property.c"

SRC_DIR=./
DST_DIR=/usr/src/linux

sync()
{
    path="${1}"
    dir=$(dirname ${path})
    file=$(basename ${path})

    rsync -a "${SRC_DIR}/${dir}/${file}" "${DST_DIR}/${dir}/${file}"
}

for i in ${items}; do
    sync "${i}"
done

