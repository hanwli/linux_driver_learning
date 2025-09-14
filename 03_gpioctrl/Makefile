obj-m := gpioctrl.o
KBUILD_CFLAGS += -save-temps=obj

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

print:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) print-vars