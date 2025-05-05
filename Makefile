
obj-m := st7735s_driver.o
# st7735s_driver-objs := st7735s_driver.o st7735s_fb.o

KERNELDIR ?= $(KDIR)

ifeq ($(KERNELDIR),)
$(error KERNELDIR is not set. Please set it before running make.)
endif

all default: modules

install: modules_install

modules modules_install help clean:
	$(MAKE) -C $(KERNELDIR) M=$(shell pwd) $@