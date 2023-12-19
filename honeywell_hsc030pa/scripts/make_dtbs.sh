#!/bin/bash

LINUX_SRC='/usr/src/linux'
BUILD_DIR='./'

create_devboard_dtb()
{
    rm -f "${BUILD_DIR}/am335x-boneblack.dtb"
    cpp -nostdinc -I include -I ${LINUX_SRC}/include/  -I ./ -I ${LINUX_SRC}/arch -undef -x assembler-with-cpp am335x-boneblack.dts am335x-boneblack.dts.preprocessed
    dtc -@ -I dts -O dtb -o "${BUILD_DIR}/am335x-boneblack.dtb" am335x-boneblack.dts.preprocessed
    rm -f am335x-boneblack.dts.preprocessed
}

create_dtbo()
{
    file=$1
    rm -f "${BUILD_DIR}/${file}.dtbo"
    cpp -nostdinc -I include -I ${LINUX_SRC}/include/ -I arch -undef -x assembler-with-cpp ${file}.dts "${BUILD_DIR}/${file}.dts.preprocessed"
    dtc -@ -I dts -O dtb -o "${BUILD_DIR}/${file}.dtbo" "${BUILD_DIR}/${file}.dts.preprocessed"
    rm -f "${BUILD_DIR}/${file}.dts.preprocessed"
}

mkdir -p "${BUILD_DIR}"

pushd "${LINUX_SRC}/arch/arm/boot/dts/ti/omap/" > /dev/null
create_devboard_dtb 2>/dev/null
popd > /dev/null

create_dtbo bb-spi0-hsc-00A0
create_dtbo bb-i2c2-hsc-00A0


