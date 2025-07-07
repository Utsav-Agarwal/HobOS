TOOLCHAIN=aarch64-elf-
CC = ${TOOLCHAIN}gcc
AS = ${TOOLCHAIN}as
LD = ${TOOLCHAIN}ld
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
CFLAGS = -Wall -O2 -ffreestanding

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

clean:
	rm -rf *.o *.img *.elf
