#!/bin/bash

LINUX_SRC='/usr/src/linux'
BUILD_DIR='/usr/src/lkm_sandbox/build'

create_devboard_dtb()
{
    rm -f "${BUILD_DIR}/am335x-boneblack.dtb"
    cpp -nostdinc -I include -I ${LINUX_SRC}/include/  -I ./ -I ${LINUX_SRC}/arch -undef -x assembler-with-cpp am335x-boneblack.dts am335x-boneblack.dts.preprocessed
    dtc -@ -I dts -O dtb -o "${BUILD_DIR}/am335x-boneblack.dtb" am335x-boneblack.dts.preprocessed
    rm -f am335x-boneblack.dts.preprocessed
}

create_dtbo()
{
    rm -f "${BUILD_DIR}/*.dtbo"
    dtc -@ -I dts -O dtb -o "${BUILD_DIR}/bb-i2c2-hsc-00A0.dtbo" bb-i2c2-hsc-00A0.dts
    cpp -nostdinc -I include -I ${LINUX_SRC}/include/ -I arch -undef -x assembler-with-cpp bb-spi0-hsc-00A0.dts "${BUILD_DIR}/bb-spi0-hsc-00A0.dts.preprocessed"
    dtc -@ -I dts -O dtb -o "${BUILD_DIR}/bb-spi0-hsc-00A0.dtbo" "${BUILD_DIR}/bb-spi0-hsc-00A0.dts.preprocessed"
    rm -f "${BUILD_DIR}/bb-spi0-hsc-00A0.dts.preprocessed"
}

pushd "${LINUX_SRC}/arch/arm/boot/dts/ti/omap/"
create_devboard_dtb 2>/dev/null
popd

create_dtbo

