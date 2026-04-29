// Code for eVM code generation testing
//
// Copyright (C) 2026  Timofey Titovets <nefelim4ag@gmail.com>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

// #include <stdint.h>
#include "example.h"

#define dummy() (RISC_V_EVM_CALL_N(1))
#define sum(a, b) (RISC_V_EVM_CALL_N2(2, a, b))
#define print(a) (RISC_V_EVM_CALL_N1(3, a))

__section("prog")
int task(int arg, int tskid)
{
    int array[10];
    array[arg] = arg;
    int a = array[arg] + 1 + tskid;
    while (a)
        a--;
    print(a);
    if (a == 0)
        a++;
    print(a);
    if (a <= 1)
        a++;
    print(a);
    int b = dummy();
    print(b);
    print(a);
    a = sum(a, b);
    return a;
}
