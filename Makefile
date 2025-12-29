TOOLCHAIN=aarch64-elf-
CC = ${TOOLCHAIN}gcc
AS = ${TOOLCHAIN}as
LD = ${TOOLCHAIN}ld
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
CFLAGS = -Wall -O2 -ffreestanding -g -march=armv8-a+nosimd -mgeneral-regs-only

all: clean kernel8.img

boot.o: boot.S
	${CC} ${CFLAGS} -c boot.S -o boot.o 

proc.o: proc.S
	${CC} ${CFLAGS} -c proc.S -o proc.o 

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

kernel8.img: boot.o proc.o ${OBJS}
	${LD} boot.o proc.o ${OBJS} -T linker.ld -o kernel8.elf -g
	${TOOLCHAIN}objcopy -O binary kernel8.elf kernel8.img

run:	all
	qemu-system-aarch64 -M raspi3b -serial null -serial stdio -s -kernel kernel8.img

run_dbg: all
	qemu-system-aarch64 -M raspi3b -serial null -serial stdio -s -kernel kernel8.img -d int

run_no_kernel: all
	qemu-system-aarch64 -M raspi3b -serial null -serial stdio -s -d int

clean:
	rm -rf *.o *.img *.elf
