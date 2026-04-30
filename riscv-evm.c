// Portable RISC-V eVM interpreter
//
// Copyright (C) 2026  Timofey Titovets <nefelim4ag@gmail.com>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <stdio.h>
#include <stdint.h>
#include "riscv-evm.h"

int evm_interpreter(struct evm_context *ctx) {
    const uint8_t MODE = ctx->mode;
    if (MODE == EVM_MODE_DISASM)
        printf("Run disassembler\n");
    if (MODE == EVM_MODE_INT_DEBUG)
        printf("Run interrupt debug mode\n");
    ctx->X[0] = 0; // always
    ctx->X[2] = (uint32_t) &ctx->stack[sizeof(ctx->stack)]; // stack pointer
    // Program counter
    uint8_t *PC = ctx->prog_start;
    uint32_t cycles = 0;
    for (;cycles < 2048; PC+=4, cycles++) {
        const uint32_t *ptr = (uint32_t *) PC;
        const uint8_t opcode = *ptr & 0x7f; // 6 bit
        if (MODE)
            fprintf(stdout, "%4x | ", PC - ctx->prog_start);
        if (opcode == 0x03) {
            uint8_t rd = (*ptr >> 7) & 0x1f;
            uint8_t func3 = (*ptr >> 12) & 0x3;
            uint8_t rs1 = (*ptr >> 15) & 0x1f;
            uint32_t offset = (*ptr >> 20) & 0xffff;
            if (offset & (1 << 12))
                offset |= 0xffffe000;
            switch (func3) {
                case 2: // lw
                    uint32_t *addr = (uint32_t *)(ctx->X[rs1] + offset);
                    if (MODE)
                        printf("lw x[%d] = M[x[%d] + %d] // = %p", rd, rs1, offset, addr);
                    if (MODE == EVM_MODE_DISASM) {
                        printf("\n");
                        continue;
                    }
                    ctx->X[rd] = *addr;
                    if (MODE)
                        printf(" %d\n", ctx->X[rd]);
                    break;
                default:
                    if (MODE)
                        printf("opcode 0x%02x, func3: %d - not implemented\n", opcode, func3);
                    return -1;
            }
        } else if (opcode == 0x13) {
            uint8_t rd = (*ptr >> 7) & 0x1f;
            uint8_t func3 = (*ptr >> 12) & 0x7;
            uint8_t rs1 = (*ptr >> 15) & 0x1f;
            uint32_t imm = (*ptr >> 20) & 0xffff;
            if (imm & (1 << 12))
                imm |= 0xffffe000;
            uint8_t shamt = imm & 0x1f;
            int32_t srs1 = ctx->X[rs1];
            switch (func3) {
                case 0: // addi
                    if (MODE)
                        printf("addi x[%d] = x[%d] + sext(%d)\n", rd, rs1, imm);
                    if (MODE == EVM_MODE_DISASM)
                        continue;
                    ctx->X[rd] = ctx->X[rs1] + imm;
                    break;
                case 1: // slli
                    if (MODE)
                        printf("slli x[%d] = x[%d] << %d\n", rd, rs1, shamt);
                    if (MODE == EVM_MODE_DISASM)
                        continue;
                    ctx->X[rd] = ctx->X[rs1] << shamt;
                    break;
                case 2: // slti
                    if (MODE)
                        printf("slti x[%d] = x[%d] <s sext(%d)\n", rd, rs1, imm);
                    if (MODE == EVM_MODE_DISASM)
                        continue;
                    int32_t i_imm = imm;
                    ctx->X[rd] = srs1 < i_imm;
                    break;
                case 3: // sltiu
                    if (MODE)
                        printf("sltiu x[%d] = x[%d] <u sext(%d)\n", rd, rs1, imm);
                    if (MODE == EVM_MODE_DISASM)
                        continue;
                    ctx->X[rd] = ctx->X[rs1] < imm;
                    break;
                case 4: // xori
                    if (MODE)
                        printf("xori x[%d] = x[%d] ^ sext(%d)\n", rd, rs1, imm);
                    if (MODE == EVM_MODE_DISASM)
                        continue;
                    ctx->X[rd] = ctx->X[rs1] ^ imm;
                    break;
                case 5: // srli srai
                    if (((imm >> 30) & 1) == 0) {
                        if (MODE)
                            printf("srli x[%d] = x[%d] >> %d\n", rd, rs1, shamt);
                        if (MODE == EVM_MODE_DISASM)
                            continue;
                        ctx->X[rd] = ctx->X[rs1] >> shamt;
                    } else {
                        if (MODE)
                            printf("srai x[%d] = x[%d] >>s %d\n", rd, rs1, shamt);
                        if (MODE == EVM_MODE_DISASM)
                            continue;
                        ctx->X[rd] = srs1 >> shamt;
                    }
                case 6: // ori
                    if (MODE)
                        printf("ori x[%d] = x[%d] | sext(%d)\n", rd, rs1, imm);
                    if (MODE == EVM_MODE_DISASM)
                        continue;
                    ctx->X[rd] = ctx->X[rs1] | imm;
                    break;
                case 7: // andi
                    if (MODE)
                        printf("andi x[%d] = x[%d] & sext(%d)\n", rd, rs1, imm);
                    if (MODE == EVM_MODE_DISASM)
                        continue;
                    ctx->X[rd] = ctx->X[rs1] & imm;
                    break;
                default:
                    if (MODE)
                        printf("opcode 0x%02x, func3: %d - not implemented\n", opcode, func3);
                    return -1;
            }
        } else if (opcode == 0x17) {
            uint8_t rd = (*ptr >> 7) & 0x1f;
            int32_t imm = *ptr & 0xffffff000;
            // AUIPC
            if (MODE)
                printf("auipc x[%d] = pc + sext(%d)\n", rd, imm);
            if (MODE == EVM_MODE_DISASM)
                continue;
            ctx->X[rd] = (uint32_t) (PC + imm);
        } else if (opcode == 0x23) {
            uint8_t func3 = (*ptr >> 12) & 0x7;
            uint8_t rs1 = (*ptr >> 15) & 0x1f;
            uint8_t rs2 = (*ptr >> 20) & 0x1f;
            uint32_t offset = ((*ptr >> 7) & 0x1f) | ((*ptr >> 25) << 5) ;
            if (offset & (1 << 11))
                offset |= 0xfffff000;
            switch (func3) {
                case 0: // sh
                {
                    uint8_t *addr = (uint8_t *) (ctx->X[rs1] + offset);
                    if (MODE)
                        printf("sb M[x[%d] + sext(%d)] = x[%d][7:0] // * (u8 *) %p = %d\n",
                            rs1, offset, rs2, addr, ctx->X[rs2]);
                    if (MODE == EVM_MODE_DISASM)
                        continue;
                    *addr = ctx->X[rs2];
                }
                    break;
                case 1: // sh
                {
                    uint16_t *addr = (uint16_t *) (ctx->X[rs1] + offset);
                    if (MODE)
                        printf("sh M[x[%d] + sext(%d)] = x[%d][15:0] // * (u16 *) %p = %d\n",
                            rs1, offset, rs2, addr, ctx->X[rs2]);
                    if (MODE == EVM_MODE_DISASM)
                        continue;
                    *addr = ctx->X[rs2];
                }
                    break;
                case 2: // sw
                {
                    uint32_t *addr = (uint32_t *) (ctx->X[rs1] + offset);
                    if (MODE)
                        printf("sw M[x[%d] + sext(%d)] = x[%d][31:0] // * (u32 *) %p = %d\n",
                            rs1, offset, rs2, addr, ctx->X[rs2]);
                    if (MODE == EVM_MODE_DISASM)
                        continue;
                    *addr = ctx->X[rs2];
                }
                    break;
                default:
                    if (MODE)
                        printf("opcode 0x%02x, func3: %d - not implemented\n", opcode, func3);
                    return -1;
            }
        } else if (opcode == 0x33) {
            uint8_t rd = (*ptr >> 7) & 0x1f;
            uint8_t func3 = (*ptr >> 12) & 0x7;
            uint8_t rs1 = (*ptr >> 15) & 0x1f;
            uint32_t rs2 = (*ptr >> 20) & 0x1f;
            uint8_t func7 = (*ptr >> 25);
            uint16_t func = (func7 << 3) | func3;
            int32_t srs1 = ctx->X[rs1];
            int32_t srs2 = ctx->X[rs2];
            switch (func) {
                case 0x000:
                    // add
                    if (MODE)
                        printf("add x[%d] = x[%d] + x[%d]\n", rd, rs1, rs2);
                    if (MODE == EVM_MODE_DISASM)
                        continue;
                    ctx->X[rd] = ctx->X[rs1] + ctx->X[rs2];
                    break;
                case 0x001:
                    // sll
                    if (MODE)
                        printf("sll x[%d] = x[%d] << x[%d]\n", rd, rs1, rs2);
                    if (MODE == EVM_MODE_DISASM)
                        continue;
                    ctx->X[rd] = ctx->X[rs1] << (ctx->X[rs2] & 0x1f);
                case 0x002:
                    // slt
                    if (MODE)
                        printf("slt x[%d] = bool(x[%d] <s x[%d])\n", rd, rs1, rs2);
                    if (MODE == EVM_MODE_DISASM)
                        continue;
                    ctx->X[rd] = srs1 < srs2;
                    break;
                case 0x003:
                    // sltu
                    if (MODE)
                        printf("sltu x[%d] = bool(x[%d] <u x[%d])\n", rd, rs1, rs2);
                    if (MODE == EVM_MODE_DISASM)
                        continue;
                    ctx->X[rd] = ctx->X[rs1] < ctx->X[rs2];
                    break;
                case 0x004:
                    // xor
                    if (MODE)
                        printf("xor x[%d] = x[%d] ^ x[%d]\n", rd, rs1, rs2);
                    if (MODE == EVM_MODE_DISASM)
                        continue;
                    ctx->X[rd] = ctx->X[rs1] ^ ctx->X[rs2];
                case 0x005:
                    // slr
                    if (MODE)
                        printf("slr x[%d] = x[%d] >> x[%d]\n", rd, rs1, rs2);
                    if (MODE == EVM_MODE_DISASM)
                        continue;
                    ctx->X[rd] = ctx->X[rs1] >> (ctx->X[rs2] & 0x1f);
                case 0x006:
                    // or
                    if (MODE)
                        printf("or x[%d] = x[%d] | x[%d]\n", rd, rs1, rs2);
                    if (MODE == EVM_MODE_DISASM)
                        continue;
                    ctx->X[rd] = ctx->X[rs1] | ctx->X[rs2];
                case 0x007:
                    // and
                    if (MODE)
                        printf("and x[%d] = x[%d] & x[%d]\n", rd, rs1, rs2);
                    if (MODE == EVM_MODE_DISASM)
                        continue;
                    ctx->X[rd] = ctx->X[rs1] & ctx->X[rs2];
                case 0x105:
                    // sra
                    if (MODE)
                        printf("slr x[%d] = x[%d] >>s x[%d]\n", rd, rs1, rs2);
                    if (MODE == EVM_MODE_DISASM)
                        continue;
                    ctx->X[rd] = srs1 >> (ctx->X[rs2] & 0x1f);
                case 0x180:
                    // sub
                    if (MODE)
                        printf("sub x[%d] = x[%d] - x[%d]\n", rd, rs1, rs2);
                    if (MODE == EVM_MODE_DISASM)
                        continue;
                    ctx->X[rd] = ctx->X[rs1] - ctx->X[rs2];
                    break;
                default:
                    if (MODE)
                        printf("opcode 0x%02x, func3: %d func7: %d - not implemented\n", opcode, func3, func7);
                    return -1;
            }
        } else if (opcode == 0x63) {
            uint8_t func3 = (*ptr >> 12) & 0x7;
            uint8_t rs1 = (*ptr >> 15) & 0x1f;
            uint8_t rs2 = (*ptr >> 20) & 0x1f;
            uint32_t offset =
                (((*ptr >> 31) & 0x1)  << 12) |  // imm[12]
                (((*ptr >> 25) & 0x3f) << 5)  |  // imm[10:5]
                (((*ptr >>  8) & 0xf)  << 1)  |  // imm[4:1]
                (((*ptr >>  7) & 0x1)  << 11);   // imm[11]
            if (offset & (1 << 12))
                offset |= 0xffffe000;
            uint32_t urs1 = ctx->X[rs1];
            uint32_t urs2 = ctx->X[rs2];
            int32_t srs1 = ctx->X[rs1];
            int32_t srs2 = ctx->X[rs2];
            switch (func3) {
                case 0: // beq
                    if (MODE)
                        printf("beq if (x[%d] == x[%d]) pc += sext(%d)) // ",
                            rs1, rs2, offset);
                    if (MODE == EVM_MODE_DISASM) {
                        printf("\n");
                        continue;
                    }
                    printf("%d == %d\n", ctx->X[rs1], ctx->X[rs2]);
                    if (ctx->X[rs1] == ctx->X[rs2])
                        PC += offset - 4;
                    break;
                case 1: // bne
                    if (MODE)
                        printf("bne if (x[%d] != x[%d]) pc += sext(%d)) // ",
                            rs1, rs2, offset);
                    if (MODE == EVM_MODE_DISASM) {
                        printf("\n");
                        continue;
                    }
                    printf("%d != %d\n", ctx->X[rs1], ctx->X[rs2]);
                    if (ctx->X[rs1] != ctx->X[rs2])
                        PC += offset - 4;
                    break;
                case 4: // blt
                    if (MODE)
                        printf("blt if (x[%d] < x[%d]) pc += sext(%d))\n",
                            rs1, rs2, offset);
                    if (MODE == EVM_MODE_DISASM)
                        continue;
                    if (srs1 < srs2)
                        PC += offset - 4;
                    break;
                case 5: // bge
                    if (MODE)
                        printf("bge if (x[%d] >= x[%d]) pc += sext(%d))\n",
                            rs1, rs2, offset);
                    if (MODE == EVM_MODE_DISASM)
                        continue;
                    if (srs1 >= srs2)
                        PC += offset - 4;
                    break;
                case 6: // bltu
                    if (MODE)
                        printf("bltu if (x[%d] < x[%d]) pc += sext(%d))\n",
                            rs1, rs2, offset);
                    if (MODE == EVM_MODE_DISASM)
                        continue;
                    if (urs1 < urs2)
                        PC += offset - 4;
                    break;
                case 7: // bgeu
                    if (MODE)
                        printf("bltu if (x[%d] >= x[%d]) pc += sext(%d))\n",
                            rs1, rs2, offset);
                    if (MODE == EVM_MODE_DISASM)
                        continue;
                    if (urs1 >= urs2)
                        PC += offset - 4;
                    break;
            }
        } else if (opcode == 0x67) {
            // uint8_t rd = (*ptr >> 7) & 0x1f;
            uint8_t func3 = (*ptr >> 12) & 0x3;
            // uint8_t rs1 = (*ptr >> 15) & 0x1f;
            // uint32_t rs2 = (*ptr >> 20) & 0x1f;
            // uint32_t offset = (*ptr >> 20) & 0xffff;
            // if (offset & (1 << 11))
            //     offset |= 0xfffff000;
            switch (func3) {
                case 0: // jalr
                    if (MODE)
                        printf("jalr // return \n");
                    return 0;
                default:
                    if (MODE)
                        printf("opcode 0x%02x, func3: %d - not implemented\n", opcode, func3);
                    return -1;
            }
        } else if (opcode == 0x6f) {
            // JAL
            uint8_t rd = (*ptr >> 7) & 0x1f;
            uint32_t offset =
                (((*ptr >> 31) & 0x1)  << 20) |  // imm[20]
                (((*ptr >> 21) & 0x3ff)<< 1)  |  // imm[10:1]
                (((*ptr >> 20) & 0x1)  << 11) |  // imm[11]
                (((*ptr >> 12) & 0xff) << 12);   // imm[19:12]
            if (offset & (1 << 20))
                offset |= 0xffe00000;
            if (MODE)
                printf("jal x[%d] = pc+4; pc += sext(%d)\n", rd, offset);
            if (MODE == EVM_MODE_DISASM)
                continue;
            if (rd) // x0 is readonly, and always zero, it is a special case
                ctx->X[rd] = (uint32_t)(PC + 4);
            PC += offset - 4;
            continue;
        } else if (opcode == 0x73) {
            uint32_t *a0 = &ctx->X[10];
            uint32_t a1 = ctx->X[11];
            uint32_t a2 = ctx->X[12];
            uint32_t a3 = ctx->X[13];
            uint32_t a4 = ctx->X[14];
            uint32_t a5 = ctx->X[15];
            if (MODE)
                printf("ecall args a5: 0x%x, a 0:%x 1:%x 2:%x 3:%x 4:%x\n",
                       a5, *a0, a1, a2, a3, a4);
            if (MODE == EVM_MODE_DISASM)
                continue;
            platform_ecall(a5, a0, a1, a2, a3, a4);
        } else {
            if (MODE)
                printf("opcode 0x%02x - not implemented\n", opcode);
            return -1;
        }
    }
    return 0;
}
