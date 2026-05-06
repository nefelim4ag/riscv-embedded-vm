// Code for eVM code generation testing
//
// Copyright (C) 2026  Timofey Titovets <nefelim4ag@gmail.com>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

// #include <stdint.h>
#include <stdint.h>
#include "example.h"

#define dummy() (RISC_V_EVM_CALL_N(1))
// #define sum(a, b) (RISC_V_EVM_CALL_N2(2, a, b))
// #define print(a) (RISC_V_EVM_CALL_N1(3, a))

#define _sendf(oid, rlen, rdata) (RISC_V_EVM_CALL_N3(2, oid, rlen, rdata))

static inline void
sendf(uint8_t oid, uint8_t rlen, uint8_t *rdata) {
    _sendf(oid, rlen, rdata);
}

__attribute__((noinline)) int sum1(int a, int b) {
    return a + b;
}

__attribute__((noinline)) int sum(int a, int b) {
    uint32_t s = sum1(a, b);
    return a + b + s;
}

__section(".start")
int task(uint32_t *args)
{
    return sum(args[0], args[1]);
}
