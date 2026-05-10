// Portable RISC-V eVM interpreter
//
// Copyright (C) 2026  Timofey Titovets <nefelim4ag@gmail.com>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <stdint.h>
#include "riscv-evm.h"

static uint8_t *
lx(uint8_t *PC, uint32_t *X, uint32_t inst, uint8_t MODE)
{
    uint8_t rd = (inst >> 7) & 0x1f;
    uint8_t func3 = (inst >> 12) & 0x7;
    uint8_t rs1 = (inst >> 15) & 0x1f;
    uint32_t offset = (inst >> 20) & 0xfff;
    if (inst & 0x80000000)
        offset |= 0xfffff000;
    uint8_t *mem = (uint8_t *)(X[rs1] + offset);
    uint8_t width = 1 << (func3 & 0x3); // 1, 2, 4 bytes
    uint8_t is_signed = !(func3 & 0x4);

    if (MODE) {
        static const char *names[] = {"lb","lh","lw","?","lbu","lhu"};
        evm_print("%s x[%d] = M[x[%d] + %d] // = %p", names[func3],
            rd, rs1, offset, mem);
    }
    if (MODE == EVM_MODE_DISASM) {
        goto out;
    }

    uint32_t val = 0;
    for (int i = 0; i < width; i++)
        val |= (uint32_t)mem[i] << (i * 8);

    uint32_t sign_bit = 1u << (width * 8 - 1);
    if (is_signed && val & sign_bit)
        val |= ~(sign_bit - 1);

    X[rd] = val;
out:
    if (MODE)
        evm_print(" %d\n", X[rd]);
    return PC + 4;
}

static uint8_t *
alu(uint8_t *PC, uint32_t *X, uint32_t inst, uint8_t MODE)
{
    uint8_t rd = (inst >> 7) & 0x1f;
    uint8_t func3 = (inst >> 12) & 0x7;
    uint8_t rs1 = (inst >> 15) & 0x1f;
    uint32_t imm = (inst >> 20) & 0xfff;
    if (inst & 0x80000000)
        imm |= 0xfffff000;
    uint8_t shamt = imm & 0x1f;
    int32_t srs1 = X[rs1];
    switch (func3) {
        case 0: // addi
            if (MODE)
                evm_print("addi x[%d] = x[%d] (0x%08x) + sext(%d) //", rd, rs1, X[rs1], (int32_t)imm);
            if (MODE == EVM_MODE_DISASM) {
                evm_print("\n");
                goto out;
            }
            X[rd] = X[rs1] + imm;
            if (MODE)
                evm_print( " %d + %d = %d\n", X[rd], X[rs1], imm);
            break;
        case 1: // slli
            if (MODE)
                evm_print("slli x[%d] = x[%d] << %d\n", rd, rs1, shamt);
            if (MODE == EVM_MODE_DISASM)
                goto out;
            X[rd] = X[rs1] << shamt;
            break;
        case 2: // slti
            if (MODE)
                evm_print("slti x[%d] = x[%d] <s sext(%d)\n", rd, rs1, imm);
            if (MODE == EVM_MODE_DISASM)
                goto out;
            int32_t i_imm = imm;
            X[rd] = srs1 < i_imm;
            break;
        case 3: // sltiu
            if (MODE)
                evm_print("sltiu x[%d] = x[%d] <u sext(%d)\n", rd, rs1, imm);
            if (MODE == EVM_MODE_DISASM)
                goto out;
            X[rd] = X[rs1] < imm;
            break;
        case 4: // xori
            if (MODE)
                evm_print("xori x[%d] = x[%d] ^ sext(%d)\n", rd, rs1, imm);
            if (MODE == EVM_MODE_DISASM)
                goto out;
            X[rd] = X[rs1] ^ imm;
            break;
        case 5: // srli srai
            if ((inst & 0x40000000) == 0) {
                if (MODE)
                    evm_print("srli x[%d] = x[%d] >> %d //", rd, rs1, shamt);
                if (MODE == EVM_MODE_DISASM) {
                    evm_print("\n");
                    goto out;
                }
                X[rd] = X[rs1] >> shamt;
                if (MODE)
                    evm_print("%d >> %d = %d\n", srs1, shamt, X[rd]);
            } else {
                if (MODE)
                    evm_print("srai x[%d] = x[%d] >>s %d //", rd, rs1, shamt);
                if (MODE == EVM_MODE_DISASM) {
                    evm_print("\n");
                    goto out;
                }
                X[rd] = srs1 >> shamt;
                if (MODE)
                    evm_print("%d >> %d = %d\n", srs1, shamt, X[rd]);
            }
            break;
        case 6: // ori
            if (MODE)
                evm_print("ori x[%d] = x[%d] | sext(%d)\n", rd, rs1, imm);
            if (MODE == EVM_MODE_DISASM)
                goto out;
            X[rd] = X[rs1] | imm;
            break;
        case 7: // andi
            if (MODE)
                evm_print("andi x[%d] = x[%d] & sext(%d)\n", rd, rs1, imm);
            if (MODE == EVM_MODE_DISASM)
                goto out;
            X[rd] = X[rs1] & imm;
            break;
    }
out:
    return PC + 4;
}

static uint8_t *
sx(uint8_t *PC, uint32_t *X, uint32_t inst, uint8_t MODE)
{
    uint8_t rd = (inst >> 7) & 0x1f;
    uint8_t func3 = (inst >> 12) & 0x7;
    uint8_t rs1 = (inst >> 15) & 0x1f;
    uint32_t rs2 = (inst >> 20) & 0x1f;
    int32_t srs1 = X[rs1];
    int32_t srs2 = X[rs2];
    switch (func3) {
        case 0:
            if (inst & 0x40000000) {
                // sub
                if (MODE)
                    evm_print("sub x[%d] = x[%d] - x[%d]\n", rd, rs1, rs2);
                if (MODE == EVM_MODE_DISASM)
                    goto out;
                X[rd] = X[rs1] - X[rs2];
                break;
            }
            // add
            if (MODE)
                evm_print("add x[%d] = x[%d] + x[%d]\n", rd, rs1, rs2);
            if (MODE == EVM_MODE_DISASM)
                goto out;
            X[rd] = X[rs1] + X[rs2];
            break;
        case 1:
            // sll
            if (MODE)
                evm_print("sll x[%d] = x[%d] << x[%d]\n", rd, rs1, rs2);
            if (MODE == EVM_MODE_DISASM)
                goto out;
            X[rd] = X[rs1] << (X[rs2] & 0x1f);
            break;
        case 2:
            // slt
            if (MODE)
                evm_print("slt x[%d] = bool(x[%d] <s x[%d])\n", rd, rs1, rs2);
            if (MODE == EVM_MODE_DISASM)
                goto out;
            X[rd] = srs1 < srs2;
            break;
        case 3:
            // sltu
            if (MODE)
                evm_print("sltu x[%d] = bool(x[%d] <u x[%d])\n", rd, rs1, rs2);
            if (MODE == EVM_MODE_DISASM)
                goto out;
            X[rd] = X[rs1] < X[rs2];
            break;
        case 4:
            // xor
            if (MODE)
                evm_print("xor x[%d] = x[%d] ^ x[%d]\n", rd, rs1, rs2);
            if (MODE == EVM_MODE_DISASM)
                goto out;
            X[rd] = X[rs1] ^ X[rs2];
            break;
        case 5:
            if (inst & 0x40000000) {
                // sra
                if (MODE)
                    evm_print("slr x[%d] = x[%d] >>s x[%d]\n", rd, rs1, rs2);
                if (MODE == EVM_MODE_DISASM)
                    goto out;
                X[rd] = srs1 >> (X[rs2] & 0x1f);
                break;
            }
            // slr
            if (MODE)
                evm_print("slr x[%d] = x[%d] >> x[%d]\n", rd, rs1, rs2);
            if (MODE == EVM_MODE_DISASM)
                goto out;
            X[rd] = X[rs1] >> (X[rs2] & 0x1f);
            break;
        case 6:
            // or
            if (MODE)
                evm_print("or x[%d] = x[%d] | x[%d]\n", rd, rs1, rs2);
            if (MODE == EVM_MODE_DISASM)
                goto out;
            X[rd] = X[rs1] | X[rs2];
            break;
        case 7:
            // and
            if (MODE)
                evm_print("and x[%d] = x[%d] & x[%d]\n", rd, rs1, rs2);
            if (MODE == EVM_MODE_DISASM)
                goto out;
            X[rd] = X[rs1] & X[rs2];
            break;
    }
out:
    return PC+4;
}

static uint8_t *
bxx(uint8_t *PC, uint32_t *X, uint32_t inst, uint8_t MODE)
{
    uint8_t func3 = (inst >> 12) & 0x7;
    uint8_t rs1 = (inst >> 15) & 0x1f;
    uint8_t rs2 = (inst >> 20) & 0x1f;
    uint32_t offset =
        (((inst >> 25) & 0x3f) << 5)  |  // imm[10:5]
        (((inst >>  8) & 0xf)  << 1)  |  // imm[4:1]
        (((inst >>  7) & 0x1)  << 11);   // imm[11]
    if (inst & 0x80000000)
        offset |= 0xfffff000;
    uint32_t urs1 = X[rs1];
    uint32_t urs2 = X[rs2];
    int32_t srs1 = X[rs1];
    int32_t srs2 = X[rs2];
    switch (func3) {
        case 0: // beq
            if (MODE)
                evm_print(
                    "beq if (x[%d] == x[%d]) pc += sext(%d)) // %d == %d\n",
                    rs1, rs2, offset, X[rs1], X[rs2]);
            if (MODE == EVM_MODE_DISASM)
                goto out;
            if (X[rs1] == X[rs2])
                return PC + offset;
            break;
        case 1: // bne
            if (MODE)
                evm_print(
                    "bne if (x[%d] != x[%d]) pc += sext(%d)) // %d != %d\n",
                    rs1, rs2, offset, X[rs1], X[rs2]);
            if (MODE == EVM_MODE_DISASM)
                goto out;
            if (X[rs1] != X[rs2])
                return PC + offset;
            break;
        case 4: // blt
            if (MODE)
                evm_print(
                    "blt if (x[%d] < x[%d]) pc += sext(%d)) // %d < %d\n",
                    rs1, rs2, offset, srs1, srs2);
            if (MODE == EVM_MODE_DISASM)
                goto out;
            if (srs1 < srs2)
                return PC + offset;
            break;
        case 5: // bge
            if (MODE)
                evm_print(
                    "bge if (x[%d] >= x[%d]) pc += sext(%d)) // %d >= %d\n",
                    rs1, rs2, offset, srs1, srs2);
            if (MODE == EVM_MODE_DISASM)
                goto out;
            if (srs1 >= srs2)
                return PC + offset;
            break;
        case 6: // bltu
            if (MODE)
                evm_print(
                    "bltu if (x[%d] < x[%d]) pc += sext(%d)) // %d < %d\n",
                    rs1, rs2, offset, urs1, urs2);
            if (MODE == EVM_MODE_DISASM)
                goto out;
            if (urs1 < urs2)
                return PC + offset;
            break;
        case 7: // bgeu
            if (MODE)
                evm_print(
                    "bgeu if (x[%d] >= x[%d]) pc += sext(%d)) // %d >= %d\n",
                    rs1, rs2, offset, urs1, urs2);
            if (MODE == EVM_MODE_DISASM)
                goto out;
            if (urs1 >= urs2)
                return PC + offset;
            break;
    }
out:
    return PC + 4;
}

static uint8_t *
sw(uint8_t *PC, uint32_t *X, uint32_t inst, uint8_t MODE)
{
    uint8_t func3 = (inst >> 12) & 0x7;
    uint8_t rs1 = (inst >> 15) & 0x1f;
    uint8_t rs2 = (inst >> 20) & 0x1f;
    uint32_t offset =
        ((inst >> 7) & 0x1f) |
        (((inst >> 25) & 0x7f) << 5);
    if (inst & 0x80000000)
        offset |= 0xfffff000;
    uint8_t *mem = (uint8_t *)(X[rs1] + offset);
    uint8_t width = 1 << (func3 & 0x3); // 1, 2, 4 bytes
    if (MODE) {
        static const char *names[] = {"sb","sh","sw"};
        evm_print("%s M[x[%d] + sext(%d)] = x[%d][%d:0] // * (u8 *) %p = %d\n",
            names[func3], rs1, offset, rs2, width * 8 -1, mem, X[rs2]);
    }
    if (MODE == EVM_MODE_DISASM)
        goto out;
    for (int i = 0; i < width; i++)
        mem[i] = X[rs2] >> (i * 8);
out:
    return PC + 4;
}

static uint8_t *
auipc(uint8_t *PC, uint32_t *X, uint32_t inst, uint8_t MODE)
{
    uint8_t rd = (inst >> 7) & 0x1f;
    int32_t imm = inst & 0xffffff000;
    // AUIPC
    if (MODE)
        evm_print("auipc x[%d] = pc + sext(%d)\n", rd, imm);
    if (MODE == EVM_MODE_DISASM)
        goto out;
    X[rd] = (uint32_t) (PC + imm);
out:
    return PC + 4;
}

static uint8_t *
lui(uint8_t *PC, uint32_t *X, uint32_t inst, uint8_t MODE)
{
    // LUI
    uint8_t rd = (inst >> 7) & 0x1f;
    int32_t imm = inst & 0xffffff000;
    if (MODE)
        evm_print("lui x[%d] = sext(%d)\n", rd, imm);
    if (MODE == EVM_MODE_DISASM)
        goto out;
    X[rd] = imm;
out:
    return PC + 4;
}

static uint8_t *
jalr(uint8_t *PC, uint32_t *X, uint32_t inst, uint8_t MODE)
{
        // JALR
    uint8_t rd = (inst >> 7) & 0x1f;
    uint8_t rs1 = (inst >> 15) & 0x1f;
    uint32_t offset = (inst >> 20) & 0xfff;
    if (inst & 0x80000000)
        offset |= 0xfffff000;
    if (MODE)
        evm_print(
            "jalr t=pc+4; pc=(x[%d]+sext(%d))&∼1; x[%d]=t\n",
            rs1, offset, rd);
    if (MODE == EVM_MODE_DISASM)
        goto out;
    uint32_t t = (uint32_t) PC + 4;
    PC = (uint8_t *) ((X[rs1] + offset) & ~1);
    if (rd)
        X[rd] = t;
    return PC;
out:
    return PC + 4;
}

static uint8_t *
jal(uint8_t *PC, uint32_t *X, uint32_t inst, uint8_t MODE)
{
    // JAL
    uint8_t rd = (inst >> 7) & 0x1f;
    uint32_t offset =
        (((inst >> 21) & 0x3ff)<< 1)  |  // imm[10:1]
        (((inst >> 20) & 0x1)  << 11) |  // imm[11]
        (((inst >> 12) & 0xff) << 12);   // imm[19:12]
    if (inst & 0x80000000)
        offset |= 0xfff00000;
    if (MODE)
        evm_print("jal x[%d] = pc+4; pc += sext(%d)\n", rd, offset);
    if (MODE == EVM_MODE_DISASM)
        goto out;
    if (rd) // x0 is readonly, and always zero, it is a special case
        X[rd] = (uint32_t)(PC + 4);
    return PC + offset;
out:
    return PC + 4;
}

static uint8_t *
ecall(uint8_t *PC, uint32_t *X, uint32_t inst, uint8_t MODE)
{
    uint16_t id = inst >> 20;
    if (MODE)
        evm_print(
            "ecall id: %d a0 %x, a1 %x, a2 %x, a3 %x, a4 %x\n",
                id, X[10], X[11], X[12], X[13], X[14]);
    if (MODE != EVM_MODE_DISASM)
        platform_ecall(id, &X[10]);
    return PC + 4;
}

int
evm_interpreter(uint8_t *prog_start, uint8_t *end,
                uint8_t mode, struct evm_args *args)
{
    const uint8_t MODE = mode;
    if (MODE == EVM_MODE_DISASM)
        evm_print("Run disassembler\n");
    if (MODE == EVM_MODE_INT_DEBUG)
        evm_print("Run interpreter debug mode\n");
    uint32_t X[16];
    uint8_t stack[256];
    X[0] = 0; // always
    X[1] = (uint32_t) end; // dummy return point
    X[2] = (uint32_t) &stack[sizeof(stack)]; // stack pointer
    X[10] = args->a0;
    X[11] = args->a1;
    X[12] = args->a2;
    X[13] = args->a3;
    X[14] = args->a4;
    // Program counter
    uint8_t *PC = prog_start;
    while (PC < end) {
        const uint32_t *ptr = (uint32_t *) PC;
        const uint8_t opcode = *ptr & 0x7f; // 6 bit
        if (MODE)
            evm_print("%4x (0x%02x) | ", PC - prog_start, opcode);
        switch (opcode) {
            case 0x03: PC = lx(PC, X, *ptr, MODE); break;
            case 0x13: PC = alu(PC, X, *ptr, MODE); break;
            case 0x17: PC = auipc(PC, X, *ptr, MODE); break;
            case 0x23: PC = sw(PC, X, *ptr, MODE); break;
            case 0x33: PC = sx(PC, X, *ptr, MODE); break;
            case 0x37: PC = lui(PC, X, *ptr, MODE); break;
            case 0x63: PC = bxx(PC, X, *ptr, MODE); break;
            case 0x67: PC = jalr(PC, X, *ptr, MODE); break;
            case 0x6f: PC = jal(PC, X, *ptr, MODE); break;
            case 0x73: PC = ecall(PC, X, *ptr, MODE); break;
            default:
                if (MODE)
                    evm_print("opcode 0x%02x - not implemented\n", opcode);
                return -1;
        }
    }
    args->a0 = X[10];
    args->a1 = X[11];
    args->a2 = X[12];
    args->a3 = X[13];
    args->a4 = X[14];
    return 0;
}
