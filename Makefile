
obj-m += hsc030pa.o hsc030pa_i2c.o hsc030pa_spi.o
KBUILD_CFLAGS += -Wall
PWD := $(CURDIR)
#RELEASE := $(shell uname -r)
RELEASE := 6.7.0-rc2+
LINUX_SRC = /usr/src/linux

SRC := $(patsubst %.o,%.c,${obj-m})

all: $(SRC)
	@make -C /lib/modules/$(RELEASE)/build M=$(PWD) modules

clean:
	@rm -f *.o *.ko .*.cmd *.mod *.mod.c .*.o.d modules.order Module.symvers depend

tags: $(SRC)
	@bash scripts/create_tags.sh $(SRC)

dtbs:
	@bash scripts/make_dtbs.sh

