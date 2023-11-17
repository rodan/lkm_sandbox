#!/bin/bash

# chip variants based on the manufacturer datasheet
# obtained via `pdftotext -layout`
input_file='hsc_variants.txt'

create_c_file()
{
    cat << EOF

struct hsc_range_config {
	char name[HSC_RANGE_STR_LEN];
	int pmin;
	int pmax;
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
        echo -e "\t{.name = \"${name}\", .pmin = ${min}, .pmax = ${max} },"
    done | column -t -R 7,10 | sed 's| \.|.|g;s| = |=|g;s| }|}|;s|^|\t|'
    echo '};'

    cat << EOF

enum hsc_variant {
EOF
    i=0
    cat "${input_file}" | while read -r line; do
        echo "${i}%6" | bc | grep -q '^0$' && echo -en '\t'
        name=$(echo "${line}" | awk '{ print $1}')
        id=$(echo "HSC${name}" | sed 's|\.|_|')
        echo -n " ${id},"
        echo "${i}%6" | bc | grep -q '^5$' && echo ''
        i=$((i+1))
    done
    echo -e '\n};'

    cat << EOF

static const struct of_device_id hsc_of_match[] = {
EOF
    i=0
    cat "${input_file}" | while read -r line; do
        echo "${i}%2" | bc | grep -q '^0$' && echo -en '\t'
        name=$(echo "${line}" | awk '{ print $1}')
        id=$(echo "HSC${name}" | sed 's|\.|_|')
        id_low="hsc$(echo ${name} | tr '[:upper:]' '[:lower:]')"
        echo -n "{ .compatible = \"honeywell,${id_low}\",}, "
        echo "${i}%2" | bc | grep -q '^1$' && echo ''
        i=$((i+1))
    done
    echo -e '\t{}'
    echo '};'

    cat << EOF

static const struct i2c_device_id hsc_id[] = {
EOF
    i=0
    cat "${input_file}" | while read -r line; do
        echo "${i}%2" | bc | grep -q '^0$' && echo -en '\t'
        name=$(echo "${line}" | awk '{ print $1}')
        id=$(echo "HSC${name}" | sed 's|\.|_|')
        id_low="hsc$(echo ${name} | tr '[:upper:]' '[:lower:]')"
        echo -n "{ \"${id_low}\", ${id} }, "
        echo "${i}%2" | bc | grep -q '^1$' && echo ''
        i=$((i+1))
    done
    echo -e '\t{}'
    echo '};'
}

create_c_file | sed 's/[ \t]*$//' > out.c


