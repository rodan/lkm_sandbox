
obj-m += abp060mg.o abp060mg_i2c.o abp060mg_spi.o
KBUILD_CFLAGS += -Wall
PWD := $(CURDIR)
LINUX_SRC = /usr/src/linux

SRC := $(patsubst %.o,%.c,${obj-m})

all: $(SRC)
	@make -C $(LINUX_SRC) M=$(PWD) modules

clean:
	@rm -f *.o *.ko .*.cmd *.mod *.mod.c .*.o.d modules.order Module.symvers depend

dtbs:
	@bash scripts/make_dtbs.sh

