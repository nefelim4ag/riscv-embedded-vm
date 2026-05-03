// Code for eVM code generation testing
//
// Copyright (C) 2026  Timofey Titovets <nefelim4ag@gmail.com>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

// #include <stdint.h>
#include <stdint.h>
#include "example.h"

#define dummy() (RISC_V_EVM_CALL_N(1))
#define sum(a, b) (RISC_V_EVM_CALL_N2(2, a, b))
#define print(a) (RISC_V_EVM_CALL_N1(3, a))

__section("prog")
int task(int arg)
{
    uint8_t array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int a = array[arg] + 1;
    // while (a)
    //     a--;
    print(a);
    if (a == 0)
        a++;
    print(a);
    if (a <= 1)
        a++;
    a = a ^ 0xffff0000;
    print(a);
    int b = dummy();
    print(b);
    print(a);
    a = sum(a, b);
    a = a ^ 0xffff0000;
    return a;
}
