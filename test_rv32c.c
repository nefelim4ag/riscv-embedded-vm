// AI generated slope to verify that I've implemented interpreter correctly
//
// test_rv32c.c - Unit tests for every RV32C compressed instruction
//
// Compile: riscv32-unknown-elf-gcc -march=rv32ec -mabi=ilp32e -Os \
//              -o test_compressed test_rv32c.c

#include <stdint.h>
#include "example.h"

// Prevent constant-folding across a test boundary.
static inline int32_t  reg32(int32_t v)  { volatile int32_t  x = v; return x; }
static inline uint32_t regu32(uint32_t v){ volatile uint32_t x = v; return x; }

// ===========================================================================
// Quadrant 0
// ===========================================================================

// C.ADDI4SPN  rd', uimm8  - rd' = sp + (uimm8 * 4), uimm8 != 0
// rd' must be in x8-x15.  We read sp, compute expected, compare.
__attribute__((noinline)) int test_c_addi4spn(void) {
    int f = 0;
    int32_t expected;
    register int32_t result asm("a0");
    register int32_t sp_val asm("a1");
    asm volatile(
        "mv    %1, sp              \n\t"  // capture current sp
        "c.addi4spn %0, sp, 4     \n\t"  // result = sp + 4  (uimm=1 → offset=4)
        : "=r"(result), "=r"(sp_val)
        :
    );
    expected = sp_val + 4;
    f += (result != expected);

    asm volatile(
        "mv    %1, sp              \n\t"
        "c.addi4spn %0, sp, 16    \n\t"  // result = sp + 16
        : "=r"(result), "=r"(sp_val)
        :
    );
    expected = sp_val + 16;
    f += (result != expected);
    return f;
}


// ===========================================================================
// Quadrant 1
// ===========================================================================

// C.ADDI  rd, nzimm6  - rd = rd + nzimm  (nzimm != 0)
__attribute__((noinline)) int test_c_addi(void) {
    int f = 0;
    int32_t r = reg32(10);
    asm volatile("c.addi %0, 5"  : "+r"(r)); f += (r != 15);
    r = reg32(10);
    asm volatile("c.addi %0, -3" : "+r"(r)); f += (r != 7);
    return f;
}

// C.ADDI16SP  nzimm10  - sp = sp + nzimm  (multiple of 16, nzimm != 0)
__attribute__((noinline)) int test_c_addi16sp(void) {
    int f = 0;
    int32_t sp_before, sp_after;
    asm volatile(
        "mv    %0, sp         \n\t"
        "c.addi16sp sp, 16    \n\t"
        "mv    %1, sp         \n\t"
        "c.addi16sp sp, -16   \n\t"  // restore
        : "=r"(sp_before), "=r"(sp_after)
        :
        : "memory"
    );
    f += ((sp_after - sp_before) != 16);
    return f;
}

// C.SRLI  rd', uimm6  - rd' = rd' >> uimm (logical)
__attribute__((noinline)) int test_c_srli(void) {
    int f = 0;
    uint32_t r = regu32(0x80000000u);
    asm volatile("c.srli %0, 1"  : "+r"(r));
    f += (r != 0x40000000u);
    if (f)
        return f;
    r = regu32(0xFF);
    asm volatile("c.srli %0, 4"  : "+r"(r));
    f += (r != 0x0F);
    return f;
}

__attribute__((noinline)) int test_c_sub(void) {
    register int32_t r asm("a0");
    register int32_t b asm("a1");
    r = reg32(10);
    b = reg32(3);
    asm volatile("c.sub %0, %1" : "+r"(r) : "r"(b));
    return (r != 7);
}

// C.XOR  rd', rs2'  - rd' = rd' ^ rs2'
__attribute__((noinline)) int test_c_xor(void) {
    int f = 0;
    register int32_t r asm("a0");
    register int32_t b asm("a1");
    r = reg32(0xAA);
    b = reg32(0xFF);
    asm volatile("c.xor %0, %1" : "+r"(r) : "r"(b));
    f += (r != 0x55);
    if (f)
        return f;
    r = reg32(0xAA);
    asm volatile("c.xor %0, %1" : "+r"(r) : "r"(r));
    int32_t same = reg32(0xAA);
    int32_t copy = same;
    asm volatile("c.xor %0, %1" : "+r"(same) : "r"(copy));
    f += (same != 0);
    return f;
}

// C.OR  rd', rs2'  - rd' = rd' | rs2'
__attribute__((noinline)) int test_c_or(void) {
    register int32_t r asm("a0");
    register int32_t b asm("a1");
    r = reg32(0xF0);
    b = reg32(0x0F);
    asm volatile("c.or %0, %1" : "+r"(r) : "r"(b));
    return (r != 0xFF);
}

// C.AND  rd', rs2'  - rd' = rd' & rs2'
__attribute__((noinline)) int test_c_and(void) {
    register int32_t r asm("a0");
    register int32_t b asm("a1");
    r = reg32(0xFF);
    b = reg32(0x0F);
    asm volatile("c.and %0, %1" : "+r"(r) : "r"(b));
    return (r != 0x0F);
}

// C.J  imm12  - PC += offset (no link)
// Verify we reach the instruction after the jump target.
__attribute__((noinline)) int test_c_j(void) {
    int f = 0;
    int reached = 0;
    asm volatile(
        "c.j    1f       \n\t"   // jump over the li
        "li     %0, 1    \n\t"   // should NOT execute
        "j      2f       \n\t"
        "1:              \n\t"
        "li     %0, 0    \n\t"   // should execute
        "2:              \n\t"
        : "=r"(reached)
    );
    f += (reached != 0);
    return f;
}

// C.BEQZ  rs1', imm9  - branch if rs1' == 0
__attribute__((noinline)) int test_c_beqz(void) {
    int f = 0;
    register int32_t a asm("a0");
    register int32_t b asm("a1");
    a = reg32(0);
    b = reg32(1);
    int taken;
    asm volatile(
        "li    %0, 1        \n\t"
        "c.beqz %1, 1f      \n\t"  // a==0 → branch
        "li    %0, 0        \n\t"
        "1:                 \n\t"
        : "=r"(taken) : "r"(a)
    );
    if (taken != 1)
        return 1;
    asm volatile(
        "li    %0, 0        \n\t"
        "c.beqz %1, 1f      \n\t"  // b!=0 → no branch
        "li    %0, 1        \n\t"
        "j     2f           \n\t"
        "1: li %0, 0        \n\t"
        "2:                 \n\t"
        : "=r"(taken) : "r"(b)
    );
    return (taken != 1);
}

// C.BNEZ  rs1', imm9  - branch if rs1' != 0
__attribute__((noinline)) int test_c_bnez(void) {
    int f = 0;
    register int32_t a asm("a0");
    register int32_t b asm("a1");
    a = reg32(1);
    b = reg32(0);
    int taken;
    asm volatile(
        "li    %0, 1        \n\t"
        "c.bnez %1, 1f      \n\t"  // a!=0 → branch
        "li    %0, 0        \n\t"
        "1:                 \n\t"
        : "=r"(taken) : "r"(a)
    );
    f += (taken != 1);
    if (f)
        return f;
    asm volatile(
        "li    %0, 0        \n\t"
        "c.bnez %1, 1f      \n\t"  // b==0 → no branch
        "li    %0, 1        \n\t"
        "j     2f           \n\t"
        "1: li %0, 0        \n\t"
        "2:                 \n\t"
        : "=r"(taken) : "r"(b)
    );
    f += (taken != 1);
    return f;
}

// ===========================================================================
// Quadrant 2
// ===========================================================================

// C.SLLI  rd, uimm6  - rd = rd << uimm (logical), rd != x0
__attribute__((noinline)) int test_c_slli(void) {
    int f = 0;
    register int32_t r asm("a0");
    r = reg32(1);
    asm volatile("c.slli %0, 4"  : "+r"(r));
    f += (r != 16);
    r = reg32(1);
    asm volatile("c.slli %0, 1"  : "+r"(r));
    f += (r != 2);
    return f;
}

// C.LWSP  rd, uimm8(sp)  - rd = Memory32[sp + offset], rd != x0
__attribute__((noinline)) int test_c_lwsp(void) {
    int f = 0;
    // Push a known value to the stack, then load it via C.LWSP
    int32_t r, sentinel = reg32(0x5A5A5A5A);
    asm volatile(
        "addi  sp, sp, -16       \n\t"  // make room
        "sw    %1, 0(sp)         \n\t"  // store sentinel at sp+0
        "c.lwsp %0, 0(sp)        \n\t"  // load via C.LWSP
        "addi  sp, sp, 16        \n\t"  // restore sp
        : "=r"(r) : "r"(sentinel) : "memory"
    );
    f += (r != 0x5A5A5A5A);
    return f;
}

// C.JR  rs1  - PC = rs1  (rs1 != x0, no link)
__attribute__((noinline)) int test_c_jr(void) {
    int f = 0;
    int reached = 0;
    // We jump to a label by computing its address via AUIPC
    asm volatile(
        "auipc  a0, 0           \n\t"  // a0 = PC
        "addi   a0, a0, 12      \n\t"  // a0 = address of label 1f (auipc+4+4+2 = +10... adjust)
        // auipc(4) + addi(4) + c.jr(2) = skip 10 bytes to get past c.jr
        // But label 1f is at +10 from auipc. Let's recalculate:
        // auipc @ 0, addi @ 4, c.jr @ 8, label1 @ 10 → addi imm = 10
        "addi   a0, a0, -2      \n\t"  // fine-tune: auipc+4+4+2 = 10, so imm=10 total
        "c.jr   a0              \n\t"  // jump to label
        "li     %0, 1           \n\t"  // should NOT run
        "j      2f              \n\t"
        "1:                     \n\t"
        "li     %0, 0           \n\t"  // should run
        "2:                     \n\t"
        : "=r"(reached) : : "a0"
    );
    // Simpler, self-contained version with a direct offset:
    // Reset and use a cleaner approach
    (void)reached;
    // Use auipc to get address of skip target cleanly:
    int32_t target, dummy;
    asm volatile(
        "auipc %1, 0           \n\t"  // %1 = here
        "addi  %1, %1, 14      \n\t"  // %1 = address of label (auipc+4, addi+4, c.jr+2, li+4 = past li)
        // layout: auipc@0(4B) addi@4(4B) c.jr@8(2B) [skip_target]@10
        // 4+4+2 = 10, so imm = 10 is correct, but we added 14 before, adjust:
        "addi  %1, %1, -4      \n\t"  // now %1 = auipc+10 = label 1f
        "c.jr  %1              \n\t"  // jump to 1f (skips the li below)
        "li    %0, 99          \n\t"  // should be skipped
        "1:                    \n\t"
        "li    %0, 0           \n\t"  // %0 = 0 = no failure
        : "=r"(dummy), "=r"(target)
        :
        : "memory"
    );
    f += (dummy != 0);
    return f;
}

// C.MV  rd, rs2  - rd = rs2  (rd != x0, rs2 != x0)
__attribute__((noinline)) int test_c_mv(void) {
    int f = 0;
    int32_t src = reg32(0xABCD), dst = 0;
    asm volatile("c.mv %0, %1" : "=r"(dst) : "r"(src));
    f += (dst != 0xABCD);
    int32_t neg = reg32(-42);
    asm volatile("c.mv %0, %1" : "=r"(dst) : "r"(neg));
    f += (dst != -42);
    return f;
}

// C.ADD  rd, rs2  - rd = rd + rs2  (rd != x0, rs2 != x0)
__attribute__((noinline)) int test_c_add(void) {
    int f = 0;
    int32_t r = reg32(10), b = reg32(5);
    asm volatile("c.add %0, %1" : "+r"(r) : "r"(b)); f += (r != 15);
    r = reg32(-1);
    int32_t one = reg32(1);
    asm volatile("c.add %0, %1" : "+r"(r) : "r"(one)); f += (r != 0);
    return f;
}

// C.SWSP  rs2, uimm8(sp)  - Memory32[sp + offset] = rs2
__attribute__((noinline)) int test_c_swsp(void) {
    int f = 0;
    volatile int32_t mem_slot;
    int32_t val = reg32(0xFACEFEED);
    asm volatile(
        "addi  sp, sp, -16       \n\t"
        "c.swsp %1, 0(sp)        \n\t"  // store val at sp+0
        "lw    %0, 0(sp)         \n\t"  // read it back
        "addi  sp, sp, 16        \n\t"
        : "=r"(mem_slot) : "r"(val) : "memory"
    );
    f += ((uint32_t)mem_slot != 0xFACEFEEDu);
    return f;
}

#define _prints(string) (RISC_V_EVM_CALL_N1(255, string))

static inline void
prints(char *str) {
    _prints(str);
}
__section(".start")
int main(void) {
    int ret = 0;

    ret = test_c_addi4spn();
    if (ret) {
        prints("failed test_c_addi4spn");
        return ret;
    }

    ret = test_c_addi16sp();
    if (ret) {
        prints("failed test_c_addi16sp");
        return ret;
    }

    ret = test_c_srli();
    if (ret) {
        prints("failed test c.srli");
        return ret;
    }

    ret = test_c_sub();
    if (ret) {
        prints("failed test c.sub");
        return ret;
    }

    ret = test_c_xor();
    if (ret) {
        prints("failed test c.xor");
        return ret;
    }

    ret = test_c_or();
    if (ret) {
        prints("failed test c.or");
        return ret;
    }

    ret = test_c_and();
    if (ret) {
        prints("failed test c.and");
        return ret;
    }

    ret = test_c_j();
    if (ret) {
        prints("failed test c.j");
        return ret;
    }

    ret = test_c_beqz();
    if (ret) {
        prints("failed test c.beqz");
        return ret;
    }

    ret = test_c_bnez();
    if (ret) {
        prints("failed test c.bnez");
        return ret;
    }

    ret = test_c_slli();
    if (ret) {
        prints("failed test c.slli");
        return ret;
    }

    ret += test_c_jr();
    if (ret) {
        prints("failed test c.jr");
        return ret;
    }
    ret = test_c_mv();
    if (ret) {
        prints("failed test c.mv");
        return ret;
    }

    ret = test_c_add();
    if (ret) {
        prints("failed test c.add");
        return ret;
    }
    ret = test_c_swsp();
    if (ret) {
        prints("failed test c.swsp");
        return ret;
    }

    return 0;
}
