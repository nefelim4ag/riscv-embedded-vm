#ifndef RISCV_EVM
#define RISCV_EVM
#include <stdint.h>

// Runtime
#define EVM_MODE_INT 0
#define EVM_MODE_INT_DEBUG 1
#define EVM_MODE_DISASM 2

struct evm_context {
    uint8_t *prog_start;
    uint32_t prog_size;
    uint8_t mode;
};

void evm_print(const char *fmt, ...);
int evm_interpreter(struct evm_context *ctx, uint32_t *a0);

// Platform helpers
void
platform_ecall(uint32_t id, uint32_t *a0, uint32_t a1,
               uint32_t a2, uint32_t a3, uint32_t a4);

#endif // RISCV_EVM
