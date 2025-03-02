build:
	aarch64-elf-as -c boot.S -o boot.o; aarch64-elf-gcc -ffreestanding -c kernel.c -o kernel.o -O2 -Wall -Wextra; 
	aarch64-elf-gcc -T linker.ld -o myos.elf -ffreestanding -O2 -nostdlib boot.o kernel.o -lgcc; 
	aarch64-elf-objcopy myos.elf -O binary kernel8.img

clean:
	rm -rf *.o *.img *.elf
