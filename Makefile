VM=qemu-system-aarch64
SCRIPTS=scripts
TOOLCHAIN_PATH=aarch64-toolchain
TOOLCHAIN=${TOOLCHAIN_PATH}/bin/aarch64-none-elf-
CC = ${TOOLCHAIN}gcc
AS = ${TOOLCHAIN}as
LD = ${TOOLCHAIN}ld
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
CFLAGS = -Wall -O2 -ffreestanding -g -march=armv8-a+nosimd -mgeneral-regs-only -Iinclude

all: clean kernel8.img

install: install-packages install-toolchain install-ci

#debian only
install-packages:
	sudo apt install make git gcc perl -y

install-toolchain:
	wget https://developer.arm.com/-/media/Files/downloads/gnu/15.2.rel1/binrel/arm-gnu-toolchain-15.2.rel1-x86_64-aarch64-none-elf.tar.xz
	mkdir -p ${TOOLCHAIN_PATH}
	tar -xvf arm-gnu-toolchain-15.2.rel1-x86_64-aarch64-none-elf.tar.xz -C ${TOOLCHAIN_PATH}
	mv aarch64-toolchain/arm-gnu-toolchain-15.2.rel1-x86_64-aarch64-none-elf/ tmp
	rm -rf ${TOOLCHAIN_PATH}
	mv tmp ${TOOLCHAIN_PATH}
	rm -rf *.xz

install-vm:
	sudo apt install ${VM}

install-ci: install-toolchain
	rm -f checkpatch.pl
	wget -P ${SCRIPTS}/ https://raw.githubusercontent.com/torvalds/linux/master/scripts/checkpatch.pl 
	chmod +x ${SCRIPTS}/checkpatch.pl
	chmod +x ${SCRIPTS}/checkpatch.sh

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
	${VM} -M raspi3b -serial null -serial stdio -s -kernel kernel8.img

run_dbg: all
	${VM} -M raspi3b -serial null -serial stdio -s -kernel kernel8.img -d int

run_no_kernel: all
	${VM} -M raspi3b -serial null -serial stdio -s -d int

clean:
	rm -rf *.o *.img *.elf

cleanall: clean
	rm -rf *.log

run_checkpatch:
	./${SCRIPTS}/checkpatch.sh ${SCRIPTS}

ci: kernel8.img run_checkpatch
