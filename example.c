// Code for eVM code generation testing
//
// Copyright (C) 2026  Timofey Titovets <nefelim4ag@gmail.com>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

// #include <stdint.h>
#include <stdint.h>
#include "example.h"

// #define dummy() (RISC_V_EVM_CALL_N(1))
// #define sum(a, b) (RISC_V_EVM_CALL_N2(2, a, b))
// #define print(a) (RISC_V_EVM_CALL_N1(3, a))

#define _sendf(oid, rlen, rdata) (RISC_V_EVM_CALL_N3(2, oid, rlen, rdata))

static inline void
sendf(uint8_t oid, uint8_t rlen, uint8_t *rdata) {
    _sendf(oid, rlen, rdata);
}

__section("prog")
void task(uint32_t *args, uint8_t *data)
{
    uint8_t oid = args[0];
    uint8_t data_len = args[1];
    sendf(oid, data_len, data);
}
