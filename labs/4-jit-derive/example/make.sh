arm-none-eabi-gcc -O2 -Wall -nostdlib -nostartfiles -ffreestanding  -march=armv6 -std=gnu99 -c add.c 
arm-none-eabi-ld add.o -T ./memmap -o add.elf 
arm-none-eabi-objdump -d add.o 
arm-none-eabi-objdump -d add.elf 

