// AI generated slope to verify that I've implemented interpreter correctly

#include <stdint.h>
#include "example.h"

static int failures = 0;

#define CHECK(name, got, expected)          \
    do {                                    \
        if ((got) != (expected)) {          \
            failures++;                     \
        }                                   \
    } while (0)

// ---------------------------------------------------------------------------
// Helpers — force values into registers without the compiler folding them.
// volatile prevents constant-propagation across the barrier.
// ---------------------------------------------------------------------------
static inline int32_t  reg32(int32_t v)  { volatile int32_t  x = v; return x; }
static inline uint32_t regu32(uint32_t v){ volatile uint32_t x = v; return x; }

// ===========================================================================
// Integer Register-Immediate
// ===========================================================================

// ADDI  rd, rs1, imm   — rd = rs1 + imm
__attribute__((noinline)) int test_addi(void) {
    int f = 0;
    int32_t r;
    int32_t a = reg32(10);
    asm volatile("addi %0, %1, 5"  : "=r"(r) : "r"(a)); CHECK("ADDI+",  r,  15); f += (r != 15);
    asm volatile("addi %0, %1, -3" : "=r"(r) : "r"(a)); CHECK("ADDI-",  r,   7); f += (r != 7);
    asm volatile("addi %0, %1, 0"  : "=r"(r) : "r"(a)); CHECK("ADDI0",  r,  10); f += (r != 10);
    return f;
}

// SLTI  rd, rs1, imm   — rd = (rs1 < imm) ? 1 : 0  (signed)
__attribute__((noinline)) int test_slti(void) {
    int f = 0;
    int32_t r;
    int32_t a = reg32(5);
    asm volatile("slti %0, %1, 10" : "=r"(r) : "r"(a)); f += (r != 1);
    asm volatile("slti %0, %1, 5"  : "=r"(r) : "r"(a)); f += (r != 0);
    asm volatile("slti %0, %1, 1"  : "=r"(r) : "r"(a)); f += (r != 0);
    // negative rs1 < positive imm
    int32_t b = reg32(-1);
    asm volatile("slti %0, %1, 0"  : "=r"(r) : "r"(b)); f += (r != 1);
    return f;
}

// SLTIU  rd, rs1, imm  — rd = ((uint)rs1 < (uint)imm) ? 1 : 0
__attribute__((noinline)) int test_sltiu(void) {
    int f = 0;
    int32_t r;
    uint32_t a = regu32(5);
    asm volatile("sltiu %0, %1, 10" : "=r"(r) : "r"(a)); f += (r != 1);
    asm volatile("sltiu %0, %1, 5"  : "=r"(r) : "r"(a)); f += (r != 0);
    // 0xFFFFFFFF is a large unsigned value
    uint32_t b = regu32(0xFFFFFFFFu);
    asm volatile("sltiu %0, %1, 1"  : "=r"(r) : "r"(b)); f += (r != 0);
    return f;
}

// XORI  rd, rs1, imm
__attribute__((noinline)) int test_xori(void) {
    int f = 0;
    int32_t r;
    int32_t a = reg32(0xF0);
    asm volatile("xori %0, %1, 0x0F" : "=r"(r) : "r"(a)); f += (r != 0xFF);
    asm volatile("xori %0, %1, -1"   : "=r"(r) : "r"(a)); f += (r != ~0xF0);
    asm volatile("xori %0, %1, 0"    : "=r"(r) : "r"(a)); f += (r != 0xF0);
    return f;
}

// ORI  rd, rs1, imm
__attribute__((noinline)) int test_ori(void) {
    int f = 0;
    int32_t r;
    int32_t a = reg32(0xF0);
    asm volatile("ori %0, %1, 0x0F" : "=r"(r) : "r"(a)); f += (r != 0xFF);
    asm volatile("ori %0, %1, 0"    : "=r"(r) : "r"(a)); f += (r != 0xF0);
    return f;
}

// ANDI  rd, rs1, imm
__attribute__((noinline)) int test_andi(void) {
    int f = 0;
    int32_t r;
    int32_t a = reg32(0xFF);
    asm volatile("andi %0, %1, 0x0F" : "=r"(r) : "r"(a)); f += (r != 0x0F);
    asm volatile("andi %0, %1, 0"    : "=r"(r) : "r"(a)); f += (r != 0);
    asm volatile("andi %0, %1, -1"   : "=r"(r) : "r"(a)); f += (r != 0xFF);
    return f;
}

// SLLI  rd, rs1, shamt  — logical left shift
__attribute__((noinline)) int test_slli(void) {
    int f = 0;
    int32_t r;
    int32_t a = reg32(1);
    asm volatile("slli %0, %1, 1"  : "=r"(r) : "r"(a)); f += (r != 2);
    asm volatile("slli %0, %1, 4"  : "=r"(r) : "r"(a)); f += (r != 16);
    asm volatile("slli %0, %1, 31" : "=r"(r) : "r"(a)); f += ((uint32_t)r != 0x80000000u);
    return f;
}

// SRLI  rd, rs1, shamt  — logical right shift
__attribute__((noinline)) int test_srli(void) {
    int f = 0;
    int32_t r;
    uint32_t a = regu32(0x80000000u);
    asm volatile("srli %0, %1, 1"  : "=r"(r) : "r"(a)); f += ((uint32_t)r != 0x40000000u);
    asm volatile("srli %0, %1, 31" : "=r"(r) : "r"(a)); f += (r != 1);
    asm volatile("srli %0, %1, 0"  : "=r"(r) : "r"(a)); f += ((uint32_t)r != 0x80000000u);
    return f;
}

// SRAI  rd, rs1, shamt  — arithmetic right shift (sign-extending)
__attribute__((noinline)) int test_srai(void) {
    int f = 0;
    int32_t r;
    int32_t a = reg32(-8);  // 0xFFFFFFF8
    asm volatile("srai %0, %1, 1"  : "=r"(r) : "r"(a)); f += (r != -4);
    asm volatile("srai %0, %1, 3"  : "=r"(r) : "r"(a)); f += (r != -1);
    int32_t b = reg32(8);
    asm volatile("srai %0, %1, 1"  : "=r"(r) : "r"(b)); f += (r != 4);
    return f;
}

// LUI  rd, imm20  — load upper 20 bits
__attribute__((noinline)) int test_lui(void) {
    int f = 0;
    int32_t r;
    // LUI loads imm into bits[31:12], zero-fills [11:0]
    asm volatile("lui %0, 1"    : "=r"(r)); f += (r != 0x1000);
    asm volatile("lui %0, 0xFF" : "=r"(r)); f += (r != (0xFF << 12));
    return f;
}

// AUIPC  rd, imm20  — add upper immediate to PC
// We can't know PC at test time, but we can verify the low 12 bits are 0
// and that two consecutive AUIPCs differ by exactly 4 (one instruction apart).
__attribute__((noinline)) int test_auipc(void) {
    int f = 0;
    int32_t r1, r2, r3;
    asm volatile(
        "auipc %0, 0\n\t"   // r1 = PC
        "auipc %1, 0\n\t"   // r2 = PC + 4
        "auipc %2, 1\n\t"   // r3 = PC + 8 + (1 << 12)
        : "=r"(r1), "=r"(r2), "=r"(r3)
    );

    // Two consecutive auipcs must differ by exactly 4
    f += ((r2 - r1) != 4);

    // auipc with imm=1 should equal the base PC+8 plus 0x1000
    f += ((r3 - r2) != (4 + 0x1000));

    // Upper 20 bits of r3 must be upper 20 bits of (PC+8) plus 1
    f += (((r3 - r2 - 4) & 0xFFF) != 0);  // no bleed into low 12 bits
    return f;
}

// ===========================================================================
// Integer Register-Register
// ===========================================================================

// ADD
__attribute__((noinline)) int test_add(void) {
    int f = 0;
    int32_t r;
    int32_t a = reg32(10), b = reg32(3);
    asm volatile("add %0, %1, %2" : "=r"(r) : "r"(a), "r"(b)); f += (r != 13);
    int32_t c = reg32(-1), d = reg32(1);
    asm volatile("add %0, %1, %2" : "=r"(r) : "r"(c), "r"(d)); f += (r != 0);
    // overflow wraps
    int32_t e = reg32(0x7FFFFFFF), g = reg32(1);
    asm volatile("add %0, %1, %2" : "=r"(r) : "r"(e), "r"(g));
    f += ((uint32_t)r != 0x80000000u);
    return f;
}

// SUB
__attribute__((noinline)) int test_sub(void) {
    int f = 0;
    int32_t r;
    int32_t a = reg32(10), b = reg32(3);
    asm volatile("sub %0, %1, %2" : "=r"(r) : "r"(a), "r"(b)); f += (r != 7);
    asm volatile("sub %0, %1, %2" : "=r"(r) : "r"(b), "r"(a)); f += (r != -7);
    int32_t c = reg32(0);
    asm volatile("sub %0, %1, %2" : "=r"(r) : "r"(c), "r"(c)); f += (r != 0);
    return f;
}

// SLL  — shift left logical (amount = rs2[4:0])
__attribute__((noinline)) int test_sll(void) {
    int f = 0;
    int32_t r;
    int32_t a = reg32(1), b = reg32(4);
    asm volatile("sll %0, %1, %2" : "=r"(r) : "r"(a), "r"(b)); f += (r != 16);
    int32_t c = reg32(0xFF), d = reg32(8);
    asm volatile("sll %0, %1, %2" : "=r"(r) : "r"(c), "r"(d)); f += (r != 0xFF00);
    // only low 5 bits of shift used
    int32_t e = reg32(1), sh = reg32(32); // 32 & 31 = 0
    asm volatile("sll %0, %1, %2" : "=r"(r) : "r"(e), "r"(sh)); f += (r != 1);
    return f;
}

// SLT  — set less than (signed)
__attribute__((noinline)) int test_slt(void) {
    int f = 0;
    int32_t r;
    int32_t a = reg32(1), b = reg32(2);
    asm volatile("slt %0, %1, %2" : "=r"(r) : "r"(a), "r"(b)); f += (r != 1);
    asm volatile("slt %0, %1, %2" : "=r"(r) : "r"(b), "r"(a)); f += (r != 0);
    asm volatile("slt %0, %1, %2" : "=r"(r) : "r"(a), "r"(a)); f += (r != 0);
    int32_t neg = reg32(-1);
    asm volatile("slt %0, %1, %2" : "=r"(r) : "r"(neg), "r"(a)); f += (r != 1);
    return f;
}

// SLTU — set less than unsigned
__attribute__((noinline)) int test_sltu(void) {
    int f = 0;
    int32_t r;
    uint32_t a = regu32(1), b = regu32(2);
    asm volatile("sltu %0, %1, %2" : "=r"(r) : "r"(a), "r"(b)); f += (r != 1);
    asm volatile("sltu %0, %1, %2" : "=r"(r) : "r"(b), "r"(a)); f += (r != 0);
    // 0xFFFFFFFF is largest unsigned — not less than 1
    uint32_t big = regu32(0xFFFFFFFFu);
    asm volatile("sltu %0, %1, %2" : "=r"(r) : "r"(big), "r"(a)); f += (r != 0);
    // 0 < 0xFFFFFFFF unsigned
    uint32_t zero = regu32(0);
    asm volatile("sltu %0, %1, %2" : "=r"(r) : "r"(zero), "r"(big)); f += (r != 1);
    return f;
}

// XOR
__attribute__((noinline)) int test_xor(void) {
    int f = 0;
    int32_t r;
    int32_t a = reg32(0xAA), b = reg32(0xFF);
    asm volatile("xor %0, %1, %2" : "=r"(r) : "r"(a), "r"(b)); f += (r != 0x55);
    asm volatile("xor %0, %1, %2" : "=r"(r) : "r"(a), "r"(a)); f += (r != 0); // a^a=0
    return f;
}

// SRL — shift right logical
__attribute__((noinline)) int test_srl(void) {
    int f = 0;
    int32_t r;
    uint32_t a = regu32(0x80000000u), b = regu32(1);
    asm volatile("srl %0, %1, %2" : "=r"(r) : "r"(a), "r"(b));
    f += ((uint32_t)r != 0x40000000u);
    uint32_t c = regu32(0xFF), d = regu32(4);
    asm volatile("srl %0, %1, %2" : "=r"(r) : "r"(c), "r"(d)); f += (r != 0x0F);
    return f;
}

// SRA — shift right arithmetic
__attribute__((noinline)) int test_sra(void) {
    int f = 0;
    int32_t r;
    int32_t a = reg32(-16), b = reg32(2);
    asm volatile("sra %0, %1, %2" : "=r"(r) : "r"(a), "r"(b)); f += (r != -4);
    int32_t c = reg32(16), d = reg32(2);
    asm volatile("sra %0, %1, %2" : "=r"(r) : "r"(c), "r"(d)); f += (r != 4);
    return f;
}

// OR
__attribute__((noinline)) int test_or(void) {
    int f = 0;
    int32_t r;
    int32_t a = reg32(0xF0), b = reg32(0x0F);
    asm volatile("or %0, %1, %2" : "=r"(r) : "r"(a), "r"(b)); f += (r != 0xFF);
    asm volatile("or %0, %1, %2" : "=r"(r) : "r"(a), "r"(a)); f += (r != 0xF0);
    return f;
}

// AND
__attribute__((noinline)) int test_and(void) {
    int f = 0;
    int32_t r;
    int32_t a = reg32(0xFF), b = reg32(0x0F);
    asm volatile("and %0, %1, %2" : "=r"(r) : "r"(a), "r"(b)); f += (r != 0x0F);
    int32_t c = reg32(0xFF), d = reg32(0);
    asm volatile("and %0, %1, %2" : "=r"(r) : "r"(c), "r"(d)); f += (r != 0);
    return f;
}

// ===========================================================================
// Load / Store
// We store a known value, then load it back and verify.
// ===========================================================================

__attribute__((noinline)) int test_sw_lw(void) {
    int f = 0;
    volatile int32_t mem;
    int32_t val = reg32(0x12345678), r;
    asm volatile("sw %1, 0(%2)\n\t"
                 "lw %0, 0(%2)"
                 : "=r"(r) : "r"(val), "r"(&mem) : "memory");
    f += (r != 0x12345678);
    return f;
}

__attribute__((noinline)) int test_sb_lb(void) {
    int f = 0;
    volatile int32_t mem = 0;
    int32_t val = reg32(0xAB), r;
    // SB stores low byte; LB sign-extends
    asm volatile("sb %1, 0(%2)\n\t"
                 "lb %0, 0(%2)"
                 : "=r"(r) : "r"(val), "r"(&mem) : "memory");
    // 0xAB = 171, sign-extended from byte = -85
    f += (r != (int8_t)0xAB);
    return f;
}

__attribute__((noinline)) int test_sb_lbu(void) {
    int f = 0;
    volatile int32_t mem = 0;
    int32_t val = reg32(0xAB), r;
    asm volatile("sb %1, 0(%2)\n\t"
                 "lbu %0, 0(%2)"
                 : "=r"(r) : "r"(val), "r"(&mem) : "memory");
    f += (r != 0xAB); // zero-extended
    return f;
}

__attribute__((noinline)) int test_sh_lh(void) {
    int f = 0;
    volatile int32_t mem = 0;
    int32_t val = reg32(0xABCD), r;
    asm volatile("sh %1, 0(%2)\n\t"
                 "lh %0, 0(%2)"
                 : "=r"(r) : "r"(val), "r"(&mem) : "memory");
    f += (r != (int16_t)0xABCD);
    return f;
}

__attribute__((noinline)) int test_sh_lhu(void) {
    int f = 0;
    volatile int32_t mem = 0;
    int32_t val = reg32(0xABCD), r;
    asm volatile("sh %1, 0(%2)\n\t"
                 "lhu %0, 0(%2)"
                 : "=r"(r) : "r"(val), "r"(&mem) : "memory");
    f += (r != 0xABCD); // zero-extended
    return f;
}

// ===========================================================================
// Branch instructions
// We use a small asm block that branches over a "fail" path.
// ===========================================================================

// BEQ — branch if equal
__attribute__((noinline)) int test_beq(void) {
    int f = 0;
    int32_t a = reg32(5), b = reg32(5), c = reg32(6);
    int taken;

    // BEQ taken case: expect branch (taken == 1)
    asm volatile(
        "li    %0, 0       \n\t"  // default: not taken
        "beq   %1, %2, 1f  \n\t"
        "j     2f          \n\t"
        "1: li %0, 1       \n\t"  // branch fired
        "2:                \n\t"
        : "=&r"(taken) : "r"(a), "r"(b)
    );
    f += (taken != 1);  // error if branch didn't fire (a==b, so it should)

    // BEQ not-taken case: expect no branch (taken == 0)
    asm volatile(
        "li    %0, 0       \n\t"  // default: not taken
        "beq   %1, %2, 1f  \n\t"
        "j     2f          \n\t"
        "1: li %0, 1       \n\t"  // branch fired
        "2:                \n\t"
        : "=&r"(taken) : "r"(a), "r"(c)
    );
    f += (taken != 0);  // error if branch fired (a!=c, so it shouldn't)
    return f;
}

// BNE — branch if not equal
__attribute__((noinline)) int test_bne(void) {
    int f = 0;
    int32_t a = reg32(5), b = reg32(6);
    int taken;
    asm volatile(
        "li    %0, 1      \n\t"
        "bne   %1, %2, 1f \n\t"
        "li    %0, 0      \n\t"
        "1:               \n\t"
        : "=r"(taken) : "r"(a), "r"(b)
    );
    f += (taken != 1);
    asm volatile(
        "li    %0, 0      \n\t"
        "bne   %1, %2, 1f \n\t"
        "li    %0, 1      \n\t"
        "j     2f         \n\t"
        "1: li %0, 0      \n\t"
        "2:               \n\t"
        : "=r"(taken) : "r"(a), "r"(a)
    );
    f += (taken != 1);
    return f;
}

// BLT — branch if less than (signed)
__attribute__((noinline)) int test_blt(void) {
    int f = 0;
    int32_t a = reg32(-1), b = reg32(1);
    int taken;
    asm volatile(
        "li    %0, 1      \n\t"
        "blt   %1, %2, 1f \n\t"  // -1 < 1
        "li    %0, 0      \n\t"
        "1:               \n\t"
        : "=r"(taken) : "r"(a), "r"(b)
    );
    f += (taken != 1);
    asm volatile(
        "li    %0, 0      \n\t"
        "blt   %1, %2, 1f \n\t"  // 1 < -1? No
        "li    %0, 1      \n\t"
        "j     2f         \n\t"
        "1: li %0, 0      \n\t"
        "2:               \n\t"
        : "=r"(taken) : "r"(b), "r"(a)
    );
    f += (taken != 1);
    return f;
}

// BGE — branch if greater or equal (signed)
__attribute__((noinline)) int test_bge(void) {
    int f = 0;
    int32_t a = reg32(5), b = reg32(5), c = reg32(4);
    int taken;
    asm volatile(
        "li    %0, 1      \n\t"
        "bge   %1, %2, 1f \n\t"  // 5 >= 5
        "li    %0, 0      \n\t"
        "1:               \n\t"
        : "=r"(taken) : "r"(a), "r"(b)
    );
    f += (taken != 1);
    asm volatile(
        "li    %0, 1      \n\t"
        "bge   %1, %2, 1f \n\t"  // 5 >= 4
        "li    %0, 0      \n\t"
        "1:               \n\t"
        : "=r"(taken) : "r"(a), "r"(c)
    );
    f += (taken != 1);
    asm volatile(
        "li    %0, 0      \n\t"
        "bge   %1, %2, 1f \n\t"  // 4 >= 5? No
        "li    %0, 1      \n\t"
        "j     2f         \n\t"
        "1: li %0, 0      \n\t"
        "2:               \n\t"
        : "=r"(taken) : "r"(c), "r"(a)
    );
    f += (taken != 1);
    return f;
}

// BLTU — branch if less than (unsigned)
__attribute__((noinline)) int test_bltu(void) {
    int f = 0;
    uint32_t a = regu32(1), b = regu32(0xFFFFFFFFu);
    int taken;

    // 1 < 0xFFFFFFFF unsigned → branch expected
    asm volatile(
        "li    %0, 0       \n\t"
        "bltu  %1, %2, 1f  \n\t"
        "j     2f          \n\t"
        "1: li %0, 1       \n\t"
        "2:                \n\t"
        : "=&r"(taken) : "r"(a), "r"(b)
    );
    f += (taken != 1);  // error if branch didn't fire

    // 0xFFFFFFFF < 1 unsigned → branch NOT expected
    asm volatile(
        "li    %0, 0       \n\t"
        "bltu  %1, %2, 1f  \n\t"
        "j     2f          \n\t"
        "1: li %0, 1       \n\t"
        "2:                \n\t"
        : "=&r"(taken) : "r"(b), "r"(a)
    );
    f += (taken != 0);  // error if branch fired
    return f;
}

// BGEU — branch if greater or equal (unsigned)
__attribute__((noinline)) int test_bgeu(void) {
    int f = 0;
    uint32_t a = regu32(0xFFFFFFFFu), b = regu32(1);
    int taken;
    asm volatile(
        "li    %0, 1       \n\t"
        "bgeu  %1, %2, 1f  \n\t"  // 0xFFFFFFFF >= 1 unsigned
        "li    %0, 0       \n\t"
        "1:                \n\t"
        : "=r"(taken) : "r"(a), "r"(b)
    );
    f += (taken != 1);
    asm volatile(
        "li    %0, 0       \n\t"
        "bgeu  %1, %2, 1f  \n\t"  // 1 >= 0xFFFFFFFF? No
        "li    %0, 1       \n\t"
        "j     2f          \n\t"
        "1: li %0, 0       \n\t"
        "2:                \n\t"
        : "=r"(taken) : "r"(b), "r"(a)
    );
    f += (taken != 1);
    return f;
}

// ===========================================================================
// JAL
// ===========================================================================

// JAL — jump and link: rd = PC+4, PC += offset
// We verify the return address is exactly 4 bytes after the JAL.
__attribute__((noinline)) int test_jal(void) {
    int f = 0;
    int32_t ra_got, ra_expected;
    asm volatile(
        "auipc  %1, 0          \n\t"  // %1 = address of this auipc
        "addi   %1, %1, 12     \n\t"  // expected ra = auipc + 4 (addi) + 4 (jal) = +8? no:
                                       // auipc(4) + addi(4) + jal(4) → ra_expected = auipc+8
                                       // but we want ra = addr_of_instruction_after_jal
                                       // Let's compute: auipc is at offset 0, addi at +4, jal at +8
                                       // instruction after jal is at +12 → %1 = auipc_addr + 12
        "jal    %0, 1f         \n\t"  // rd=%0 = PC+4 (addr after jal = auipc+12)
        "1:                    \n\t"
        : "=r"(ra_got), "=r"(ra_expected)
        :
        : "memory"
    );
    f += (ra_got != ra_expected);
    return f;
}

#define _prints(string) (RISC_V_EVM_CALL_N1(255, string))

static inline void
prints(char *str) {
    _prints(str);
}

__section(".start")
int main(void) {
    int ret;
    ret = test_addi();
    if (ret) {
        prints("failed addi");
        return ret;
    }

    ret = test_slti();
    if (ret) {
        prints("failed slti");
        return ret;
    }

    ret = test_sltiu();
    if (ret) {
        prints("failed sltiu");
        return ret;
    }

    ret = test_xori();
    if (ret) {
        prints("failed xori");
        return ret;
    }

    ret = test_ori();
    if (ret) {
        prints("failed ori");
        return ret;
    }

    ret = test_andi();
    if (ret) {
        prints("failed andi");
        return ret;
    }

    ret = test_slli();
    if (ret) {
        prints("failed slli");
        return ret;
    }

    ret = test_srli();
    if (ret) {
        prints("failed srli");
        return ret;
    }

    ret = test_srai();
    if (ret) {
        prints("failed srai");
        return ret;
    }

    ret = test_lui();
    if (ret) {
        prints("failed lui");
        return ret;
    }

    ret = test_auipc();
    if (ret) {
        prints("failed auipc");
        return ret;
    }

    ret = test_add();
    if (ret) {
        prints("failed add");
        return ret;
    }

    ret = test_sub();
    if (ret) {
        prints("failed sub");
        return ret;
    }

    ret = test_sll();
    if (ret) {
        prints("failed sll");
        return ret;
    }

    ret = test_slt();
    if (ret) {
        prints("failed slt");
        return ret;
    }

    ret = test_sltu();
    if (ret) {
        prints("failed sltu");
        return ret;
    }

    ret = test_xor();
    if (ret) {
        prints("failed xor");
        return ret;
    }

    ret = test_srl();
    if (ret) {
        prints("failed srl");
        return ret;
    }

    ret = test_sra();
    if (ret) {
        prints("failed sra");
        return ret;
    }

    ret = test_or();
    if (ret) {
        prints("failed or");
        return ret;
    }

    ret = test_and();
    if (ret) {
        prints("failed and");
        return ret;
    }

    ret = test_sw_lw();
    if (ret) {
        prints("failed lw");
        return ret;
    }

    ret = test_sb_lb();
    if (ret) {
        prints("failed lb");
        return ret;
    }

    ret = test_sb_lbu();
    if (ret) {
        prints("failed lbu");
        return ret;
    }

    ret = test_sh_lh();
    if (ret) {
        prints("failed lh");
        return ret;
    }

    ret = test_sh_lhu();
    if (ret) {
        prints("failed lhu");
        return ret;
    }

    ret = test_beq();
    if (ret) {
        prints("failed beq");
        return ret;
    }

    ret = test_bne();
    if (ret) {
        prints("failed bne");
        return ret;
    }

    ret = test_blt();
    if (ret) {
        prints("failed blt");
        return ret;
    }

    ret = test_bge();
    if (ret) {
        prints("failed bge");
        return ret;
    }

    ret = test_bltu();
    if (ret) {
        prints("failed bltu");
        return ret;
    }

    ret = test_bgeu();
    if (ret) {
        prints("failed bgeu");
        return ret;
    }

    ret = test_jal();
    if (ret) {
        prints("failed jal");
        return ret;
    }

    // Return total failure count. 0 = all passed.
    return 0;
}
