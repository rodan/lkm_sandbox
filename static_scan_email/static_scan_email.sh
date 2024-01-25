#!/bin/bash

# Mutt-based script that performs a static scan of a patchset
#
# source: https://github.com/rodan/lkm_sandbox
# author: Petre Rodan <petre.rodan@subdimension.ro>

prefix='/tmp'
msg_file="${prefix}/foo"
msg_idx="${prefix}/msg_index"
kernel_src="/usr/src/linux-6.8"

create_makefile()
{
    dir=$1
	pushd "${dir}"
	files=$(ls -1 *.c | xargs | sed 's|\.c|\.o|g')
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
	popd
}

patch_file()
{
	patch_file="$1"
	out_dir=$(dirname "${patch_file}")
	# get all files that will get patched
	file_list=$(grep -E '^[-+]{3}.*/drivers/iio' < "${patch_file}" | sed 's|.*drivers/iio|drivers/iio|' | sort -u)
	for file in ${file_list}; do
		cp "${kernel_src}/${file}" "${out_dir}"
	done
	pushd "${out_dir}"
		patch --forward < "${patch_file}"
	popd
}

message=$(cat)

echo "${message}" > "${msg_file}"

msg_id=$(822field message-id < "${msg_file}" | sed 's|[ <>]||g')
msg_ref=$(822field references < "${msg_file}")
msg_subject=$(822field subject < "${msg_file}")
msg_date=$(822field date < "${msg_file}")

[ -z "${msg_ref}" ] && {
    # cover message, 0/x
    formatted_date=$(date -d"${msg_date}" +%y%m%d_%H%M%S)
    subsys=$(echo "${msg_subject}" | sed 's|.*\(iio:.*\):.*|\1|;s|: |_|g')
    msg_dir="${formatted_date}-${subsys}"
    echo "${msg_dir} | ${msg_id}" >> "${msg_idx}"
	out_dir="${prefix}/${formatted_date}"
    mkdir -p "${out_dir}"
    mv "${msg_file}" "${out_dir}/${msg_id}"
	patch_file "${out_dir}/${msg_id}"
	create_makefile "${out_dir}"
	exit 0
}




