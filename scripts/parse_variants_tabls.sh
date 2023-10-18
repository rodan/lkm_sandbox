#!/bin/bash

# chip variants based on the manufacturer datasheet
# obtained via `pdftotext -layout`
input_file='hsc_variants.txt'

create_c_file()
{
    cat << EOF

struct hsc_config {
	int min;
	int max;
};

// all values have been converted to pascals
static struct hsc_config hsc_config[] = {
EOF
    cat "${input_file}" | while read -r line; do
        name=$(echo "${line}" | awk '{ print $1}')
        id=$(echo "HSC${name}" | sed 's|\.|_|')
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
                mult=6895
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
        echo -e "[${id}] = { .min = ${min}, .max = ${max} },"
    done | column -t -R 6,9,12,15 | sed 's| \.|.|g;s| = |=|g;s| }|}|;s|^|\t|'
    echo '};'

    cat << EOF

enum hsc_variant {
EOF
    i=0
    cat "${input_file}" | while read -r line; do
        echo "${i}%6" | bc | grep -q '^0$' && echo -en '\t'
        name=$(echo "${line}" | awk '{ print $1}')
        id=$(echo "HSC${name}" | sed 's|\.|_|')
        echo -n " ${id}"
        echo "${i}%6" | bc | grep -q '^5$' && echo ''
        i=$((i+1))
    done
    echo -e '\n};'


    cat << EOF

static const struct i2c_device_id abp060mg_id_table[] = {
EOF
    i=0
    cat "${input_file}" | while read -r line; do
        echo "${i}%2" | bc | grep -q '^0$' && echo -en '\t'
        name=$(echo "${line}" | awk '{ print $1}')
        id=$(echo "HSC${name}" | sed 's|\.|_|')
        id_low=$(echo "${id}" | tr '[:upper:]' '[:lower:]')
        echo -n "{ \"${id_low}\", ${id} }, "
        echo "${i}%2" | bc | grep -q '^1$' && echo ''
        i=$((i+1))
    done
    echo '};'
}

create_c_file > out.c


