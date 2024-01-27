
obj-m += hsc030pa.o hsc030pa_i2c.o hsc030pa_spi.o
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

scan-build: clean
	@scan-build make
