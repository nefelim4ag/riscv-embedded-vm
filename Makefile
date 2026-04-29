CC = gcc
CFLAGS = -O2 -g -m32 -Wall

CC_VM = riscv64-linux-gnu-gcc
VM_CFLAGS = -O0 -march=rv32e -mabi=ilp32e -ffreestanding -nostdlib

all: main.bin example.bin

main.bin: main.c riscv-evm.o
	$(CC) $(CFLAGS) -o $@ $^

riscv-evm.o: riscv-evm.c riscv-evm.h
	$(CC) $(CFLAGS) -c $<

example.o: example.c
	$(CC_VM) $(VM_CFLAGS) -c -o $@ $<

example.elf: example.o rv32e.ld
	$(CC_VM) $(VM_CFLAGS) -T rv32e.ld -Wl,-n -Wl,--oformat=elf32-littleriscv -o  $@ $<

example.bin: example.elf
	riscv64-linux-gnu-objcopy -O binary $< $@

clean:
	rm -fv riscv-evm.o main example.o
