#!/bin/bash

# chip variants based on the manufacturer datasheet
# obtained via `pdftotext -layout`
IN_FILE='ssc_variants.txt'

clean_d() {
    input_d=$1
    if [ "$(echo "${input_d}" | sed 's|[0-9]||g' | wc -c)" != '1' ]; then
        err 'error: invalid input'
        exit 1
    fi
    clean_input="${input_d}"
}

conv_dh() {
    clean_d "$1"
    result=$(echo "ibase=10;obase=16;${clean_input}" | bc)
}

prettify_hex() {
    h="${1}"
    echo "$h" | tr '[:upper:]' '[:lower:]' | sed 's|^|0x|;s| | 0x|g' | xargs
}


create_c_file()
{
    echo -n 'enum hsc_variants {'
    i=0
    cat "${IN_FILE}" | while read -r line; do
        if [ $((i%4)) == 0 ]; then
            echo -en "\n\t"
        else
            echo -n ' '
        fi
        name=$(echo "${line}" | awk '{ print $1}')
        enum=$(echo "${name}" | sed 's|\.|_|;')
        conv_dh "${i}"
        num=$(prettify_hex "${result}")
        echo -n "HSC${enum} = ${num},"
        i=$((i+1))
    done
    echo -e " HSC_VARIANTS_MAX"
    echo -e '};\n'

    echo -n 'static const char * const hsc_triplet_variants[HSC_VARIANTS_MAX] = {'
    i=0
    cat "${IN_FILE}" | while read -r line; do
        if [ $((i%3)) == 0 ]; then
            echo -en "\n\t"
        else
            echo -n ' '
        fi
        name=$(echo "${line}" | awk '{ print $1}')
        enum=$(echo "${name}" | sed 's|\.|_|;')
        echo -en "[HSC${enum}] = \"${name}\","
        i=$((i+1))
    done
    echo -e '\n};\n'

    cat << EOF
/**
 * struct hsc_range_config - pressure ranges based on the nomenclature
 * @triplet: 5-char string that defines the range, measurement unit and type
 * @pmin: lowest pressure that can be measured
 * @pmax: highest pressure that can be measured
 */
struct hsc_range_config {
	const s32 pmin;
	const s32 pmax;
};

// all values have been converted to pascals
static const struct hsc_range_config hsc_range_config[HSC_VARIANTS_MAX] = {
EOF
    cat "${IN_FILE}" | while read -r line; do
        name=$(echo "${line}" | awk '{ print $1}')
        enum=$(echo "${name}" | sed 's|\.|_|;')
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
        echo -e "\t[HSC${enum}] = { .pmin = ${min}, .pmax = ${max} },"
    done | column -t -R 6,9 | sed 's| \.|.|g;s| = |=|g;s| }|}|;s|^|\t|'
    echo '};'
}

create_c_file | sed 's/[ \t]*$//' > out.c


