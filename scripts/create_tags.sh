#!/bin/bash

mkdir -p /tmp/fake_includes/gnu
touch /tmp/fake_includes/{stddef.h,stubs-soft.h,gnu/stubs-soft.h}


touch depend

makedepend -f depend  -I/usr/src/linux/include -I/usr/src/linux/arch/arm/include -I/usr/src/linux/arch/arm/include/generated/ -I/tmp/fake_includes $@

echo './' > depmod_list
grep ': ' depend | cut -d':' -f2 | sed 's/[ \t]*$//;s|^[ \t]*||g;s| |\n|g' >> depmod_list

exuberant-ctags -L depmod_list --recurse

#rm -f depend

