#!/bin/bash

# Mutt-based script that performs a static scan of a patchset
#
# dependencies: mess822
# source: https://github.com/rodan/lkm_sandbox
# author: Petre Rodan <petre.rodan@subdimension.ro>

prefix='/var/lib/sse'
msg_file="${prefix}/foo"
msg_idx="${prefix}/msg_index"
kernel_src="/usr/src/linux-6.8"

check_setup()
{
    [ ! -w "${prefix}" ] && {
        echo "make sure ${prefix} (\$prefix) directory exists and is writable by the current user"
        exit 1
    }
    return 0
}

create_makefile()
{
    dir=$1
	pushd "${dir}" >/dev/null || exit 1
    files=$(find ./ -type f -name '*.c' | sed 's|^\./||;s|\.c|\.o|g' | xargs)
    cat << EOF > "${dir}/Makefile"
obj-m += ${files}
KBUILD_CFLAGS += -Wall
PWD := \$(CURDIR)
LINUX_SRC = ${kernel_src}

SRC := \$(patsubst %.o,%.c,\${obj-m})

all: \$(SRC)
	@make -C \$(LINUX_SRC) M=\$(PWD) modules

clean:
	@rm -f *.o *.ko .*.cmd *.mod *.mod.c .*.o.d modules.order Module.symvers depend

scan-build: clean
	@scan-build make
EOF
	popd >/dev/null || exit 1
    return 0
}

get_headers()
{
    dir="$1"
	pushd "${dir}" >/dev/null || exit 1
    c_files=$(find ./ -type f -name '*.c' | sed 's|^\./||' | xargs)
    for c_file in ${c_files}; do
        header_files=$(grep '^#include' ${c_file} | grep -v '/' | cut -d'"' -f2)
        for h_file in ${header_files}; do
            find ${kernel_src}/drivers/iio/ -type f -name "${h_file}" -exec cp --no-clobber {} ./ \;
        done
    done
	popd >/dev/null || exit 1
    return 0
}

create_links()
{
    dir="$1"
    [ ! -e "${dir}/drivers" ] && ln -s ${kernel_src}/drivers "${dir}/drivers"
    [ ! -e "${dir}/include" ] && ln -s ${kernel_src}/include "${dir}/include"
    [ ! -e "${dir}/kernel" ] && ln -s ${kernel_src}/kernel "${dir}/kernel"
}

patch_file()
{
	patch_file="$1"
    echo "${patch_file}" | grep -q 'Makefile' && return
	out_dir=$(dirname "${patch_file}")
	# get all files that will get patched
	file_list=$(grep -E '^[-+]{3}.*/drivers/iio' < "${patch_file}" | sed 's|.*drivers/iio|drivers/iio|' | sort -u)
	for file in ${file_list}; do
        cp --no-clobber "${kernel_src}/${file}" "${out_dir}"
	done
	pushd "${out_dir}" >/dev/null || exit 1
		patch --batch --forward < "${patch_file}"
	popd >/dev/null || exit 1
    return 0
}

parse_msg()
{
    in_file="$1"
    msg_id=$(822field message-id < "${in_file}" | sed 's|[ <>]||g')
    msg_ref=$(822field references < "${in_file}" | sed 's|[ <>]||g')
    msg_subject=$(822field subject < "${in_file}" | sed 's|[<>]||g')
    msg_date=$(822field date < "${in_file}")
    formatted_date=$(date -d"${msg_date}" +%Y%m%d_%H%M%S)
    if echo "${msg_subject}" | grep -q 'iio:.*:'; then
        subsys=$(echo "${msg_subject}" | sed 's|.*\(iio:.*\):.*|\1|;s|: |_|g')
    else
        subsys=$(echo "${msg_subject}" | awk '{ print $NF }')
    fi
    msg_dir="${formatted_date}-${subsys}"

    echo "Subject: ${msg_subject}"

    if [ -z "${msg_ref}" ]; then
        # cover message, 0/x
        echo "${msg_dir} | ${msg_id}" >> "${msg_idx}"
        out_dir="${prefix}/${msg_dir}"
    else
        out_dir_tail=$(grep ${msg_ref} "${msg_idx}" | tail -n1 | awk '{ print $1 }')
        if [ -z "${out_dir_tail}" ]; then
            out_dir="${prefix}/${msg_dir}"
            echo "${msg_dir} | ${msg_ref}" >> "${msg_idx}"
        else
            out_dir="${prefix}/${out_dir_tail}"
        fi
    fi

    mkdir -p "${out_dir}"
    mv "${in_file}" "${out_dir}/${msg_id}"
    patch_file "${out_dir}/${msg_id}"
    get_headers "${out_dir}"
    create_makefile "${out_dir}"
    create_links "${out_dir}"
    return 0
}

check_setup
cat > "${msg_file}"
parse_msg "${msg_file}"

