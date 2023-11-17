
obj-m += hsc030pa.o hsc030pa_i2c.o hsc030pa_spi.o
KBUILD_CFLAGS += -Wall
PWD := $(CURDIR)
#RELEASE := $(shell uname -r)
RELEASE := 6.1.38+

SRC := $(patsubst %.o,%.c,${obj-m})

all: $(SRC)
	@make -C /lib/modules/$(RELEASE)/build M=$(PWD) modules

clean:
	@rm -f *.o *.ko .*.cmd *.mod *.mod.c modules.order Module.symvers depend

tags: $(SRC)
	@scripts/create_tags.sh $(SRC)

dtbo: $(SRC)
	@dtc -@ -I dts -O dtb -o bb-i2c2-hsc-00A0.dtbo bb-i2c2-hsc-00A0.dts

