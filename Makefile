
CC      = arm-linux-gcc
LD      = arm-linux-ld
AR      = arm-linux-ar
OBJCOPY = arm-linux-objcopy
OBJDUMP = arm-linux-objdump

INCLUDEDIR 	:= $(shell pwd)/include
CFLAGS 		:= -Wall -O2
CPPFLAGS   	:= -nostdinc -fno-builtin -march=armv4 -I$(INCLUDEDIR)
LDFLAGS	= -lgcc
LDFLAGS	+= -L/home/shift/arm-toolchain/s3c2440/x-tool/arm-s3c2440-linux-gnueabi/lib/gcc/arm-s3c2440-linux-gnueabi/4.8.2

export 	CC LD OBJCOPY OBJDUMP INCLUDEDIR CFLAGS CPPFLAGS AR

objs := head.o init.o nand.o interrupt.o i2c.o sd.o m41t11.o serial.o main.o lib/libc.a lib/eabi_compat.o

i2c.bin: $(objs)
	${LD} -Ti2c.lds -o i2c_elf $^ $(LDFLAGS)
	${OBJCOPY} -O binary -S i2c_elf $@
	${OBJDUMP} -D -m arm i2c_elf > i2c.dis

.PHONY : lib/libc.a
lib/libc.a:
	cd lib; make; cd ..
	
%.o:%.c
	${CC} $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

%.o:%.S
	${CC} $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	make  clean -C lib
	rm -f i2c.bin i2c_elf i2c.dis *.o
	
