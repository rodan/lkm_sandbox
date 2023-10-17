#!/bin/bash

touch depend

makedepend -f depend  -I/usr/src/linux/include -I/usr/src/linux/arch/arm/include -I/usr/src/linux/arch/arm/include/generated/ -Ifake_includes $@

echo './' > depmod_list
grep ': ' depend | cut -d':' -f2 | sed 's/[ \t]*$//;s|^[ \t]*||g;s| |\n|g' >> depmod_list

exuberant-ctags -L depmod_list --recurse

#rm -f depend

