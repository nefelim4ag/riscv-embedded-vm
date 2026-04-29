# RISC-V Embedded interpreter
My attempt to implement an analog of eBPF in the RISC-V ISA for embedded use

# example.c

Example programm

# main.c

For interpreter debbugging development

# riscv-evm.c riscv-evm.h

supposed to be library functions in the end

# example.h

supposed to be eBPF program headers
with call mappings

# Example run

Interpreter support 3 modes:
- Execution
- Execution + Debug
- Disassembler

```
make
./main ebpf.o
...
make; ./main.bin example.bin
gcc -O2 -g -m32 -Wall -c riscv-evm.c
gcc -O2 -g -m32 -Wall -o main.bin main.c riscv-evm.o
Run disassembler
   0 | addi x[2] = x[2] + sext(-64)
   4 | sw M[x[2] + sext(60)] = x[1][31:0] // * (u32 *) 0xffeb648c = 0
   8 | sw M[x[2] + sext(56)] = x[8][31:0] // * (u32 *) 0xffeb6488 = 0
   c | addi x[8] = x[2] + sext(64)
  10 | sw M[x[8] + sext(-60)] = x[10][31:0] // * (u32 *) 0xffffffc4 = 8
  14 | sw M[x[8] + sext(-64)] = x[11][31:0] // * (u32 *) 0xffffffc0 = 0
  18 | lw x[14] = M[x[8] + -60] // = 0xffffffc4
  1c | addi x[15] = x[8] + sext(-56)
  20 | slli x[14] = x[14] << 2
  24 | add x[15] = x[14] + x[15]
  28 | lw x[14] = M[x[8] + -60] // = 0xffffffc4
  2c | sw M[x[15] + sext(0)] = x[14][31:0] // * (u32 *) (nil) = 0
  30 | lw x[14] = M[x[8] + -60] // = 0xffffffc4
  34 | addi x[15] = x[8] + sext(-56)
  38 | slli x[14] = x[14] << 2
  3c | add x[15] = x[14] + x[15]
  40 | lw x[15] = M[x[15] + 0] // = (nil)
  44 | addi x[15] = x[15] + sext(1)
  48 | lw x[14] = M[x[8] + -64] // = 0xffffffc0
  4c | add x[15] = x[14] + x[15]
  50 | sw M[x[8] + sext(-12)] = x[15][31:0] // * (u32 *) 0xfffffff4 = 0
  54 | jal x[0] = pc+4; pc += sext(16)
  58 | lw x[15] = M[x[8] + -12] // = 0xfffffff4
  5c | addi x[15] = x[15] + sext(-1)
  60 | sw M[x[8] + sext(-12)] = x[15][31:0] // * (u32 *) 0xfffffff4 = 0
  64 | lw x[15] = M[x[8] + -12] // = 0xfffffff4
  68 | bne if (x[15] != x[0]) pc += sext(-16)) // 
  6c | addi x[15] = x[0] + sext(3)
...
  d0 | addi x[15] = x[10] + sext(0)
  d4 | sw M[x[8] + sext(-16)] = x[15][31:0] // * (u32 *) 0xfffffff0 = 0
  d8 | addi x[15] = x[0] + sext(3)
  dc | lw x[14] = M[x[8] + -16] // = 0xfffffff0
  e0 | addi x[10] = x[14] + sext(0)
  e4 | ecall args a5: 0x0, a 0:8 1:0 2:0 3:0 4:0
  e8 | addi x[15] = x[0] + sext(3)
  ec | lw x[14] = M[x[8] + -12] // = 0xfffffff4
  f0 | addi x[10] = x[14] + sext(0)
  f4 | ecall args a5: 0x0, a 0:8 1:0 2:0 3:0 4:0
  f8 | addi x[15] = x[0] + sext(2)
  fc | lw x[14] = M[x[8] + -12] // = 0xfffffff4
 100 | addi x[10] = x[14] + sext(0)
 104 | lw x[14] = M[x[8] + -16] // = 0xfffffff0
 108 | addi x[11] = x[14] + sext(0)
 10c | ecall args a5: 0x0, a 0:8 1:0 2:0 3:0 4:0
 110 | addi x[15] = x[10] + sext(0)
 114 | sw M[x[8] + sext(-12)] = x[15][31:0] // * (u32 *) 0xfffffff4 = 0
 118 | lw x[15] = M[x[8] + -12] // = 0xfffffff4
 11c | addi x[10] = x[15] + sext(0)
 120 | lw x[1] = M[x[2] + 60] // = 0xffeb648c
 124 | lw x[8] = M[x[2] + 56] // = 0xffeb6488
 128 | addi x[2] = x[2] + sext(64)
 12c | jalr // return 
...
Run interrupt debug mode
   0 | addi x[2] = x[2] + sext(-64)
   4 | sw M[x[2] + sext(60)] = x[1][31:0] // * (u32 *) 0xfff6b40c = 0
   8 | sw M[x[2] + sext(56)] = x[8][31:0] // * (u32 *) 0xfff6b408 = 0
   c | addi x[8] = x[2] + sext(64)
  10 | sw M[x[8] + sext(-60)] = x[10][31:0] // * (u32 *) 0xfff6b3d4 = 8
  14 | sw M[x[8] + sext(-64)] = x[11][31:0] // * (u32 *) 0xfff6b3d0 = 0
  18 | lw x[14] = M[x[8] + -60] // = 0xfff6b3d4 8
  1c | addi x[15] = x[8] + sext(-56)
  20 | slli x[14] = x[14] << 2
  24 | add x[15] = x[14] + x[15]
  28 | lw x[14] = M[x[8] + -60] // = 0xfff6b3d4 8
  2c | sw M[x[15] + sext(0)] = x[14][31:0] // * (u32 *) 0xfff6b3f8 = 8
  30 | lw x[14] = M[x[8] + -60] // = 0xfff6b3d4 8
  34 | addi x[15] = x[8] + sext(-56)
  38 | slli x[14] = x[14] << 2
  3c | add x[15] = x[14] + x[15]
  40 | lw x[15] = M[x[15] + 0] // = 0xfff6b3f8 8
  44 | addi x[15] = x[15] + sext(1)
  48 | lw x[14] = M[x[8] + -64] // = 0xfff6b3d0 0
  4c | add x[15] = x[14] + x[15]
  50 | sw M[x[8] + sext(-12)] = x[15][31:0] // * (u32 *) 0xfff6b404 = 9
  54 | jal x[0] = pc+4; pc += sext(16)
  68 | bne if (x[15] != x[0]) pc += sext(-16)) // 9 != 0
  5c | addi x[15] = x[15] + sext(-1)
  60 | sw M[x[8] + sext(-12)] = x[15][31:0] // * (u32 *) 0xfff6b404 = 8
  64 | lw x[15] = M[x[8] + -12] // = 0xfff6b404 8
  68 | bne if (x[15] != x[0]) pc += sext(-16)) // 8 != 0
  5c | addi x[15] = x[15] + sext(-1)
  60 | sw M[x[8] + sext(-12)] = x[15][31:0] // * (u32 *) 0xfff6b404 = 7
  64 | lw x[15] = M[x[8] + -12] // = 0xfff6b404 7
  68 | bne if (x[15] != x[0]) pc += sext(-16)) // 7 != 0
  5c | addi x[15] = x[15] + sext(-1)
  60 | sw M[x[8] + sext(-12)] = x[15][31:0] // * (u32 *) 0xfff6b404 = 6
  64 | lw x[15] = M[x[8] + -12] // = 0xfff6b404 6
  68 | bne if (x[15] != x[0]) pc += sext(-16)) // 6 != 0
  5c | addi x[15] = x[15] + sext(-1)
  60 | sw M[x[8] + sext(-12)] = x[15][31:0] // * (u32 *) 0xfff6b404 = 5
  64 | lw x[15] = M[x[8] + -12] // = 0xfff6b404 5
  68 | bne if (x[15] != x[0]) pc += sext(-16)) // 5 != 0
  5c | addi x[15] = x[15] + sext(-1)
  60 | sw M[x[8] + sext(-12)] = x[15][31:0] // * (u32 *) 0xfff6b404 = 4
  64 | lw x[15] = M[x[8] + -12] // = 0xfff6b404 4
  68 | bne if (x[15] != x[0]) pc += sext(-16)) // 4 != 0
  5c | addi x[15] = x[15] + sext(-1)
  60 | sw M[x[8] + sext(-12)] = x[15][31:0] // * (u32 *) 0xfff6b404 = 3
  64 | lw x[15] = M[x[8] + -12] // = 0xfff6b404 3
  68 | bne if (x[15] != x[0]) pc += sext(-16)) // 3 != 0
  5c | addi x[15] = x[15] + sext(-1)
  60 | sw M[x[8] + sext(-12)] = x[15][31:0] // * (u32 *) 0xfff6b404 = 2
  64 | lw x[15] = M[x[8] + -12] // = 0xfff6b404 2
  68 | bne if (x[15] != x[0]) pc += sext(-16)) // 2 != 0
  5c | addi x[15] = x[15] + sext(-1)
  60 | sw M[x[8] + sext(-12)] = x[15][31:0] // * (u32 *) 0xfff6b404 = 1
  64 | lw x[15] = M[x[8] + -12] // = 0xfff6b404 1
  68 | bne if (x[15] != x[0]) pc += sext(-16)) // 1 != 0
  5c | addi x[15] = x[15] + sext(-1)
  60 | sw M[x[8] + sext(-12)] = x[15][31:0] // * (u32 *) 0xfff6b404 = 0
  64 | lw x[15] = M[x[8] + -12] // = 0xfff6b404 0
  68 | bne if (x[15] != x[0]) pc += sext(-16)) // 0 != 0
  6c | addi x[15] = x[0] + sext(3)
  70 | lw x[14] = M[x[8] + -12] // = 0xfff6b404 0
  74 | addi x[10] = x[14] + sext(0)
  78 | ecall args a5: 0x3, a 0:0 1:0 2:0 3:0 4:0
print 0
...
  f8 | addi x[15] = x[0] + sext(2)
  fc | lw x[14] = M[x[8] + -12] // = 0xfff6b404 2
 100 | addi x[10] = x[14] + sext(0)
 104 | lw x[14] = M[x[8] + -16] // = 0xfff6b400 2
 108 | addi x[11] = x[14] + sext(0)
 10c | ecall args a5: 0x2, a 0:2 1:2 2:0 3:0 4:2
 110 | addi x[15] = x[10] + sext(0)
 114 | sw M[x[8] + sext(-12)] = x[15][31:0] // * (u32 *) 0xfff6b404 = 4
 118 | lw x[15] = M[x[8] + -12] // = 0xfff6b404 4
 11c | addi x[10] = x[15] + sext(0)
 120 | lw x[1] = M[x[2] + 60] // = 0xfff6b40c 0
 124 | lw x[8] = M[x[2] + 56] // = 0xfff6b408 0
 128 | addi x[2] = x[2] + sext(64)
 12c | jalr // return 
Execution result: 4
```
