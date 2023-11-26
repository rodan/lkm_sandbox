#!/bin/bash

# chip variants based on the manufacturer datasheet
# obtained via `pdftotext -layout`
input_file='ssc_variants.txt'

create_c_file()
{
    cat << EOF

struct hsc_range_config {
	char name[HSC_PRESSURE_TRIPLET_LEN];
	s32 pmin;
	u32 pmax;
};

// all values have been converted to pascals
static struct hsc_range_config hsc_range_config[] = {
EOF
    cat "${input_file}" | while read -r line; do
        name=$(echo "${line}" | awk '{ print $1}')
        #id=$(echo "HSC${name}" | sed 's|\.|_|')
        unit=$(echo "${line}" | awk '{ print $4}')
        mult=0
        case "${unit}" in
            kPa)
                mult=1000
                ;;
            MPa)
                mult=1000000
                ;;
            Pa)
                mult=1
                ;;
            mbar)
                mult=100
                ;;
            bar)
                mult=100000
                ;;
            psi)
                mult=6894.75729
                ;;
            inH2O)
                mult=249.0889
                ;;
        esac
        min=$(echo "${line}" | awk '{ print $2}')
        min=$(echo "${min}*${mult}" | bc)
        min=$(printf "%.0f" "${min}")
        max=$(echo "${line}" | awk '{ print $3}')
        max=$(echo "${max}*${mult}" | bc)
        max=$(printf "%.0f" "${max}")

        #echo "${name} ${id} ${min} ${max} ${unit}"
        echo -e "\t{.name = \"${name}\", .pmin = ${min}, .pmax = ${max} },"
    done | column -t -R 6,9 | sed 's| \.|.|g;s| = |=|g;s| }|}|;s|^|\t|'
    echo '};'
}

create_c_file | sed 's/[ \t]*$//' > out.c


