// Code to open and test RISC-V eVM programs
//
// Copyright (C) 2026  Timofey Titovets <nefelim4ag@gmail.com>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "riscv-evm.h"

static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s <name>.bin\n", prog);
    exit(1);
}

void evm_print(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

void
platform_ecall(uint32_t *regs) {
    uint16_t id = regs[0];
    uint32_t ret = 0;
    uint32_t a1 = regs[1];
    uint32_t a2 = regs[2];
    uint32_t a3 = regs[3];
    uint32_t a4 = regs[4];
    uint32_t a5 = regs[5];
    switch (id) {
        // dummy()
        case 1:
            ret = 2;
            break;
        // sum(a, b)
        case 2:
            uint8_t *data = (uint8_t *) a3;
            printf("oid: %d len: %d ", a1, a2);
            for (int i = 0; i < a2; i++) {
                printf("0x%02x ", data[i]);
            }
            printf("\n");
            break;
        case 3:
            printf("print %d\n", a1);
            return;
        default:
            printf("unknown id\n");
            break;
    }
    regs[0] = ret;
}

int main(int argc, char *argv[]) {
    if (argc != 2) usage(argv[0]);

    const char *path = argv[1];

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror(path);
        exit(1);
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        exit(1);
    }

    size_t size = st.st_size;
    if (size == 0) {
        fprintf(stderr, "%s: empty file\n", path);
        close(fd);
        exit(1);
    }

    uint8_t *base = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if (base == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    uint32_t array[] = {0, 3};
    uint32_t arg = (uint32_t) array;
    uint8_t arg2[] = {0x00, 0x12, 0x34 };
    struct evm_args args = {
        .a0 = (uint32_t) array,
        .a1 = (uint32_t) arg2,
    };
    int ret = evm_interpreter(base, size, EVM_MODE_INT_DEBUG, &args);
    if (ret < 0) {
        printf("evm_interpreter failed\n");
    }
    printf("Execution result: %d\n", args.a0);

    munmap(base, size);
    return 0;
}
