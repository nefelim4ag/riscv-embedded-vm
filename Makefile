CC = gcc
CFLAGS = -O2 -g -m32 -Wall

CC_VM = riscv64-linux-gnu-gcc
VM_CFLAGS = -O2 -march=rv32e -mabi=ilp32e -ffreestanding -nostdlib -fPIE

all: main.bin example.bin test_rv32e_base.bin

main.bin: main.c riscv-evm.o
	$(CC) $(CFLAGS) -o $@ $^

riscv-evm.o: riscv-evm.c riscv-evm.h
	$(CC) $(CFLAGS) -c $<

example.o: example.c
	$(CC_VM) $(VM_CFLAGS) -c -o $@ $<

example.elf: example.o rv32e.ld
	$(CC_VM) $(VM_CFLAGS) -T rv32e.ld -static -Wl,--build-id=none -o $@ $<

example.bin: example.elf
	riscv64-linux-gnu-objcopy -O binary $< $@

test_rv32e_base.o: test_rv32e_base.c
	$(CC_VM) $(VM_CFLAGS) -c -o $@ $<

test_rv32e_base.elf: test_rv32e_base.o rv32e.ld
	$(CC_VM) $(VM_CFLAGS) -T rv32e.ld -static -Wl,--build-id=none -o $@ $<

test_rv32e_base.bin: test_rv32e_base.elf
	riscv64-linux-gnu-objcopy -O binary $< $@

clean:
	rm -fv riscv-evm.o main example.o test_rv32e_base.o
