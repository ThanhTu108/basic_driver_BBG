# basic_driver_BBG

obj-m+=name_file.o

CROSS=/usr/bin/arm-linux-gnueabi-
KERNEL=/home/tu/BBG/linux

all:
   make ARCH=arm CROSS_COMPILE=${CROSS} -C ${KERNEL} M=$(PWD) modules
clean:
   make -C ${KERNEL} M=$(PWD) clean

